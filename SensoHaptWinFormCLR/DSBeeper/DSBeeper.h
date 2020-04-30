/* DSBeeper (and Win32Beeper)
 * 2009-01-21 - Initial version
 */

#ifndef _DSBEEPER_H
#define _DSBEEPER_H

#define _USE_MATH_DEFINES
#define _WIN32_DCOM
#include <mmreg.h>
#include <dsound.h>
#include <cmath>
#include <basetsd.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

typedef INT16 buffer_int_t;

// Note, this doesn't work on Windows Vista/2k3 x64
// Use DSBeeper instead
class Win32Beeper{
public:
	BOOL Beep(DWORD dwFreq, DWORD dwDuration){
		return ::Beep(dwFreq, dwDuration);
	}
};

class DSBeeper{
	LPDIRECTSOUNDBUFFER m_lpDSBuffer;
	LPDIRECTSOUND m_lpDS;
	DSBUFFERDESC dsbdesc;
	WAVEFORMATEX wfx;

	// These probably should not be changed
	static const INT32 MAX_FREQ = 32767;
	static const INT32 MIN_FREQ = 37;
	static const INT32 BITS_PER_SAMPLE = 16;
	static const INT32 NUM_CHANNELS = 1;
	static const INT32 SAMPLING_RATE = 44100;
	static const INT32 BUFFER_SIZE = SAMPLING_RATE * NUM_CHANNELS;
	static const INT32 SCALE = (1 << (BITS_PER_SAMPLE - 1))-1;
	// BUFFER_SIZE is just the maximum; it will not be used

	// Target frequency precision: less than 0.5% error
	//   37  Hz @ 44.1kHz sampling -> half-period of ~596 samples
	// 32.8 kHz @ 44.1kHz sampling -> half-period of ~0.67 samples
public:
	DSBeeper():m_lpDS(NULL){
		ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
		ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));

		// All we do here is create the IDirectSound object
		if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))){
			return;
		}
		if(FAILED(DirectSoundCreate(NULL, &m_lpDS, NULL))){
			m_lpDS = NULL;
			return;
		}
		if(FAILED(m_lpDS->SetCooperativeLevel(GetDesktopWindow(),
		                                      DSSCL_NORMAL))){
			m_lpDS->Release();
			m_lpDS = NULL;
			return;
		}
		
		// This won't change each to we call Beep(), so cache the values
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = NUM_CHANNELS;
		wfx.wBitsPerSample = BITS_PER_SAMPLE;
		wfx.nSamplesPerSec = SAMPLING_RATE;
		wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = 0;
		
		// Cache these values so we don't have to fill it in later
		dsbdesc.dwSize = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY
		                | DSBCAPS_CTRLFREQUENCY
		                | DSBCAPS_GLOBALFOCUS;
		dsbdesc.lpwfxFormat = &wfx;
	}
	~DSBeeper(){
		if(NULL != m_lpDS){ m_lpDS->Release(); }
		CoUninitialize();
	}
	
	// This function fails silently; if parameters are out of bounds,
	// they are clamped to the bounds.
	// If DirectSound failed to initialize, returns FALSE.
	BOOL Beep(DWORD dwFreq, DWORD dwDuration){
		if(NULL == m_lpDS){ return FALSE; }
		
		// Clamp the frequency to the acceptable range
		if(dwFreq < MIN_FREQ){ dwFreq = MIN_FREQ; }
		if(dwFreq > MAX_FREQ){ dwFreq = MAX_FREQ; }

		// 1/dwFreq = period of wave, in seconds
		// SAMPLING_RATE / dwFreq = samples per wave-period
		INT32 half_period = SAMPLING_RATE * NUM_CHANNELS / (2*dwFreq);
		// The above line introduces roundoff error, which at higher
		// frequencies is significant (>30% at the 32kHz,
		// easily above 1% in general, not good). We will fix this below.

		// If frequency too high, make sure it's not just a constant DC level
		if(half_period < 1){ half_period = 1; }
		INT32 buffer_size = 2*half_period * BITS_PER_SAMPLE/8;

		// Make buffer
		dsbdesc.dwBufferBytes = buffer_size;
		if(FAILED(m_lpDS->CreateSoundBuffer(&dsbdesc, &m_lpDSBuffer, NULL))){
			return FALSE;
		}

		// Frequency adjustment to correct for the roundoff error above.
		DWORD play_freq;
		if(FAILED(m_lpDSBuffer->GetFrequency(&play_freq))){
			m_lpDSBuffer->Release();
			return FALSE;
		}
		// When we set the half_period above, we rounded down, so if
		// we play the buffer as is, it will sound higher frequency
		// than it ought to be.
		// To compensate, we should play the buffer at a slower speed.
		// The slowdown factor is precisely the rounded-down period
		// divided by the true period:
		//   half_period / [ SAMPLING_RATE * NUM_CHANNELS / (2*dwFreq) ]
		// = 2*dwFreq*half_period / (SAMPLING_RATE * NUM_CHANNELS)
		//
		// The adjusted frequency needs to be multiplied by this factor:
		//   play_freq *= 2*dwFreq*half_period / (SAMPLING_RATE * NUM_CHANNELS)
		// To do this computation (in a way that works on 32-bit machines),
		// we cannot multiply the numerator directly, since that may
		// cause rounding problems (44100 * 2*44100*1 ~ 3.9 billion which
		// is uncomfortable close to the upper limit of 4.3 billion).
		// Therefore, we use MulDiv to safely (and efficiently) avoid any
		// problems.
		play_freq = MulDiv(play_freq, 2*dwFreq*half_period,
		                   SAMPLING_RATE * NUM_CHANNELS);
		if(FAILED(m_lpDSBuffer->SetFrequency(play_freq))){
			m_lpDSBuffer->Release();
			return FALSE;
		}

		// Write to buffer
		LPVOID lpvWrite;
		DWORD dwLen;
		if(SUCCEEDED(m_lpDSBuffer->Lock(0, buffer_size, &lpvWrite, &dwLen,
		                                NULL, NULL, DSBLOCK_ENTIREBUFFER))){
			buffer_int_t *p = (buffer_int_t*)lpvWrite;

			// Write in the square wave
			int i = half_period;
			while(i--){
				for(int j = 0; j < NUM_CHANNELS; ++j){
					*p = -SCALE; p++;
				}
			}
			i = half_period;
			while(i--){
				for(int j = 0; j < NUM_CHANNELS; ++j){
					*p = SCALE; p++;
				}
			}

			// Don't bother Sleeping if it didn't Play
			if(SUCCEEDED(m_lpDSBuffer->Unlock(lpvWrite, dwLen, NULL, NULL))){
				if(SUCCEEDED(m_lpDSBuffer->SetCurrentPosition(0))){
					if(SUCCEEDED(m_lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING))){
						Sleep(dwDuration);
						m_lpDSBuffer->Stop();
					}
				}
			}
		}

		// Delete buffer
		if(NULL != m_lpDSBuffer){ m_lpDSBuffer->Release(); }

		return TRUE;
	}
};

#endif // _DSBEEPER_H
