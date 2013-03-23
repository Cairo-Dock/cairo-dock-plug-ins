/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* based on 'aplay' from Jaroslav Kysela <perex@perex.cz>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#ifndef __FreeBSD__
#include <endian.h>
#include <byteswap.h>
#else
#include <sys/endian.h>
#endif

#include "applet-struct.h"
#include "applet-sound.h"


#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)   ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)           (v)
#define LE_INT(v)       (v)
#define BE_SHORT(v)           bswap_16(v)
#define BE_INT(v)       bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)   ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)           bswap_16(v)
#define LE_INT(v)       bswap_32(v)
#define BE_SHORT(v)           (v)
#define BE_INT(v)       (v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF        COMPOSE_ID('R','I','F','F')
#define WAV_WAVE        COMPOSE_ID('W','A','V','E')
#define WAV_FMT               COMPOSE_ID('f','m','t',' ')
#define WAV_DATA        COMPOSE_ID('d','a','t','a')
#define WAV_PCM_CODE          1

typedef struct {
	guint magic;            /* 'RIFF' */
	guint length;           /* filelen */
	guint type;       /* 'WAVE' */
} WaveHeader;

typedef struct {
	gushort format;         /* should be 1 for PCM-code */
	gushort modus;          /* 1 Mono, 2 Stereo */
	guint sample_fq;  /* frequence of sample */
	guint byte_p_sec;  // number of bytes/sec (Frequence * BytePerFrame)
	gushort byte_p_spl;     // bytes per frame (NbChannels * BitsPerSample/8)
	gushort bit_p_spl;      // bits per sample (8, 16, 24, 32)
} WaveFmtBody;

typedef struct {
	guint type;       /* 'data' */
	guint length;           /* samplecount */
} WaveChunkHeader;

#define DEFAULT_FORMAT        SND_PCM_FORMAT_U8
#define DEFAULT_SPEED         8000


static gboolean _parse_header (CDSoundFile *pSoundFile)
{
	gchar *end = pSoundFile->buffer + pSoundFile->length;
	// check the header
	if (pSoundFile->length < sizeof(WaveHeader) + sizeof (WaveChunkHeader) + sizeof (WaveFmtBody))  // minimum size.
		return FALSE;
	gchar *ptr = pSoundFile->buffer;
	WaveHeader *h = (WaveHeader *)ptr;
	if (h->magic != WAV_RIFF || h->type != WAV_WAVE)
		return FALSE;
	ptr += sizeof(WaveHeader);
     
     // check the audio format
	guint type, len;
	WaveChunkHeader *c;
	do
	{
		c = (WaveChunkHeader*) ptr;
		type = c->type;
		len = LE_INT(c->length);
		len += len % 2;
		if (type == WAV_FMT)
			break;
		ptr += sizeof (WaveChunkHeader) + len;
	} while (ptr < end);
	g_return_val_if_fail  (ptr < end, FALSE);

	ptr += sizeof (WaveChunkHeader);
	WaveFmtBody *f = (WaveFmtBody*) ptr;
	 
	if (len < sizeof(WaveFmtBody))
	{
		cd_warning ("unknown length of 'fmt ' chunk (read %u, should be %u at least)",
		len, (guint)sizeof(WaveFmtBody));
		return FALSE;
	}

	gint format = LE_SHORT(f->format);
	gint channels = LE_SHORT(f->modus);
	gint rate = LE_INT(f->sample_fq);  // frames/s
	gint bytePerSec = LE_INT(f->byte_p_sec);
	gint bytePerBloc = LE_SHORT(f->byte_p_spl);
	gint bitsPerSample = LE_SHORT(f->bit_p_spl);
	
	if (format != WAV_PCM_CODE)
	{
		cd_warning ("can't play not PCM-coded WAVE-files");
		return FALSE;
	}
	if (LE_SHORT(f->modus) < 1)
	{
		cd_warning ("can't play WAVE-files with %d tracks", LE_SHORT(f->modus));
		return FALSE;
	}
	pSoundFile->channels = LE_SHORT(f->modus);
	
      switch (LE_SHORT(f->bit_p_spl)) {
      case 8:
            if (pSoundFile->format != DEFAULT_FORMAT &&
                pSoundFile->format != SND_PCM_FORMAT_U8)
                  cd_warning ("Warning: format is changed to U8\n");
            pSoundFile->format = SND_PCM_FORMAT_U8;
            break;
      case 16:
            if (pSoundFile->format != DEFAULT_FORMAT &&
                pSoundFile->format != SND_PCM_FORMAT_S16_LE)
                  cd_warning ("Warning: format is changed to S16_LE\n");
            pSoundFile->format = SND_PCM_FORMAT_S16_LE;
            break;
      case 24:
            switch (LE_SHORT(f->byte_p_spl) / pSoundFile->channels) {
            case 3:
                  if (pSoundFile->format != DEFAULT_FORMAT &&
                      pSoundFile->format != SND_PCM_FORMAT_S24_3LE)
                        cd_warning ("Warning: format is changed to S24_3LE\n");
                  pSoundFile->format = SND_PCM_FORMAT_S24_3LE;
                  break;
            case 4:
                  if (pSoundFile->format != DEFAULT_FORMAT &&
                      pSoundFile->format != SND_PCM_FORMAT_S24_LE)
                        cd_warning ("Warning: format is changed to S24_LE\n");
                  pSoundFile->format = SND_PCM_FORMAT_S24_LE;
                  break;
            default:
                  cd_warning (" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)",
                        LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), pSoundFile->channels);
                  return FALSE;
            }
            break;
      case 32:
            pSoundFile->format = SND_PCM_FORMAT_S32_LE;
            break;
      default:
            cd_warning (" can't play WAVE-files with sample %d bits wide",
                  LE_SHORT(f->bit_p_spl));
            return FALSE;
      }

	cd_debug ("rate: %d; channels: %d; BytePerSec: %d; BytePerBloc: %d; BitsPerSample: %d", rate, channels, bytePerSec, bytePerBloc, bitsPerSample);
	pSoundFile->rate = LE_INT(rate);
	
	ptr += len;
	// check the audio data
	while (ptr < end)
	{
		c = (WaveChunkHeader*) ptr;
		type = c->type;
		len = LE_INT(c->length);
		len += len % 2;
		if (type == WAV_DATA)
			break;
		ptr += sizeof (WaveChunkHeader) + len;
	};
	g_return_val_if_fail  (ptr < end, FALSE);
	
	cd_debug ("len = %d; file size = %d", len, pSoundFile->length);
	pSoundFile->size = len;
	pSoundFile->iNbFrames = pSoundFile->size / bytePerBloc;
	pSoundFile->iBitsPerSample = bitsPerSample;
	pSoundFile->fTimelength = (double)pSoundFile->size / bytePerSec;
	ptr += sizeof (WaveChunkHeader);
	pSoundFile->data = ptr;
	
	return TRUE;
}

typedef struct {
	char a;
	char b;
	char c;
} int24;

CDSoundFile *cd_sound_load_sound_file (const gchar *cFilePath)
{
	// read the file at once
	gchar *buffer = NULL;
	gsize length = 0;
	g_file_get_contents (cFilePath,
		&buffer,
		&length,
		NULL);
	g_return_val_if_fail (buffer != NULL, NULL);
	
	CDSoundFile *pSoundFile = g_new0 (CDSoundFile, 1);
	pSoundFile->buffer = buffer;
	pSoundFile->length = length;
	pSoundFile->format = DEFAULT_FORMAT;
	pSoundFile->rate = DEFAULT_SPEED;
	pSoundFile->channels = 1;
	
	// parse the header to get the info
	_parse_header (pSoundFile);
	
	// apply volume
	if (myConfig.fVolume < .99)
	{
		int iNbSamples = pSoundFile->size * 8 / pSoundFile->iBitsPerSample;
		int i;
		/**gchar *data = pSoundFile->data;
		cd_debug ("%d samples", iNbSamples);
		gint32 sample;
		gint spl;
		gint32 *psample;
		for (i = 0; i < iNbSamples-3; i++)
		{
			psample = (gint32 *)&data[i*pSoundFile->iBitsPerSample/8];
			sample = *psample;
			spl = sample & (0xFFFFFFFF >> (32 - pSoundFile->iBitsPerSample));
			spl *= myConfig.fVolume;
			*psample = (sample & (0xFFFFFFFF << pSoundFile->iBitsPerSample)) + spl;
		}*/
		
		// .1 -> /2
		// (1 - vol) * 10 + 1
		double v = (1 - myConfig.fVolume) * 10 + 1;
		switch (pSoundFile->iBitsPerSample)
		{
			case 8:
			{
				char *data = (char*)pSoundFile->data;
				char *psample;
				for (i = 0; i < iNbSamples; i++)
				{
					psample = &data[i];
					*psample /= v;
				}
			}
			break;
			case 16:
			{
				gint16 *data = (gint16*)pSoundFile->data;
				gint16 *psample;
				for (i = 0; i < iNbSamples; i++)
				{
					psample = &data[i];
					*psample /= v;
				}
			}
			break;
			case 24:
			{
				int24 *data = (int24*)pSoundFile->data;
				int24 sample24, *psample;
				gint32 sample;
				for (i = 0; i < iNbSamples; i++)
				{
					psample = &data[i];
					sample24 = *psample;
					sample = (sample24.a<<16) + (sample24.b<<8) + sample24.c;
					sample /= v;
					sample24.a = (sample & 0x00FF0000) >> 16;
					sample24.b = (sample & 0x0000FF00) >> 8;
					sample24.c = sample & 0x000000FF;
					*psample = sample24;
				}
			}
			break;
			case 32:
			{
				gint32 *data = (gint32*)pSoundFile->data;
				gint32 *psample;
				for (i = 0; i < iNbSamples; i++)
				{
					psample = &data[i];
					*psample /= v;
				}
			}
			break;
			default:
			break;
		}
	}
	
	return pSoundFile;
}



static gboolean set_params (snd_pcm_t *handle, CDSoundFile *pSoundFile)
{
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold, stop_threshold;
	static unsigned period_time = 0;
	static unsigned buffer_time = 0;
	static snd_pcm_uframes_t chunk_size = 0;
	static size_t bits_per_sample, bits_per_frame;
	
	// initiate hardware params
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca (&params);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		cd_warning ("Broken configuration for this PCM: no configurations available");
		return FALSE;
	}
	err = snd_pcm_hw_params_set_access (handle, params,
		SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0)
	{
		cd_warning ("Access type not available");
		return FALSE;
	}

	// set format
	err = snd_pcm_hw_params_set_format(handle, params, pSoundFile->format);
	if (err < 0)
	{
		cd_warning ("Sample format non available");
		return FALSE;
	}
	// set channels number
	err = snd_pcm_hw_params_set_channels(handle, params, pSoundFile->channels);
	if (err < 0)
	{
		cd_warning ("Channels count non available");
		return FALSE;
	}

	// set rate
	rate = pSoundFile->rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	assert(err >= 0);
	if ((float)rate * 1.05 < pSoundFile->rate || (float)rate * 0.95 > pSoundFile->rate)
	{
		cd_warning ("rate is not accurate (requested = %iHz, got = %iHz)\n", pSoundFile->rate, rate);
	}
	pSoundFile->rate = rate;

	// set buffer and period time
	err = snd_pcm_hw_params_get_buffer_time_max(params,
		&buffer_time, 0);
	assert(err >= 0);
	if (buffer_time > 500000)
		buffer_time = 500000;

	period_time = buffer_time / 4;

	err = snd_pcm_hw_params_set_period_time_near(handle, params,
	&period_time, 0);
	assert(err >= 0);
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
	&buffer_time, 0);
	assert(err >= 0);

	// now set paramas to the handle
	err = snd_pcm_hw_params(handle, params);
	if (err < 0)
	{
		cd_warning ("Unable to install hw params:");
		return FALSE;
	}

	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size == buffer_size) {
		cd_warning ("Can't use period equal to buffer size (%lu == %lu)",
		chunk_size, buffer_size);
		return FALSE;
	}

	// software params
	snd_pcm_sw_params_alloca(&swparams);
	snd_pcm_sw_params_current(handle, swparams);
	n = chunk_size;
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = buffer_size;
	start_threshold = n;
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
	assert(err >= 0);
	stop_threshold = buffer_size;
	err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
	assert(err >= 0);
	
	if (snd_pcm_sw_params(handle, swparams) < 0)
	{
		cd_warning ("unable to install sw params:");
		return FALSE;
	}
	
	bits_per_sample = snd_pcm_format_physical_width(pSoundFile->format);
	bits_per_frame = bits_per_sample * pSoundFile->channels;
	
	cd_debug ("bits_per_frame: %d; rate: %d", bits_per_frame, pSoundFile->rate);
	cd_debug ("chunk_size: %d; buffer_size: %d", chunk_size, buffer_size);
	pSoundFile->iNbFrames = (pSoundFile->size * 8) / bits_per_frame;
	return TRUE;
}


static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	// close the handle
	if (pSharedMemory->handle)
		snd_pcm_close (pSharedMemory->handle);
	
	// remove the task from the list of tasks
	myData.pTasks = g_list_remove (myData.pTasks, pSharedMemory->pTask);
	g_free (pSharedMemory);
}

static gboolean _finish_play_sound (CDSharedMemory *pSharedMemory)
{
	cairo_dock_discard_task (pSharedMemory->pTask);
	return FALSE;
}

static void _play_sound_async (CDSharedMemory *pSharedMemory)
{
	CDSoundFile *pSoundFile = pSharedMemory->pSoundFile;
	g_return_if_fail (pSoundFile != NULL);
	// open the device
	snd_pcm_t *handle = NULL;
	int err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);  // open-mode = 0
	if (err < 0)
	{
		cd_warning ("audio open error: %s", snd_strerror(err));
		return;
	}
	
	// set params
	if ( !set_params (handle, pSoundFile))
		return;
	
	// play data
	ssize_t r;
	int n = pSoundFile->iNbFrames;
	gchar *buffer = pSoundFile->data;
	while (n > 0)
	{
		r = snd_pcm_writei (handle, buffer, n);
		if (r == -EAGAIN || (r >= 0 && r < n))
		{
			snd_pcm_wait (handle, 100);
		}
		else if (r == -EPIPE)  // an underrun occurred 
		{
			cd_debug ("underrun");
			snd_pcm_status_t *status;
			int res;
			snd_pcm_status_alloca(&status);
			if ((res = snd_pcm_status(handle, status))<0)
			{
				cd_warning ("status error: %s", snd_strerror(res));
				return;
			}
			snd_pcm_state_t state = snd_pcm_status_get_state(status);
			
			if (state == SND_PCM_STATE_XRUN)
			{
				if ((res = snd_pcm_prepare (handle)) < 0)
				{
					cd_warning ("prepare error: %s", snd_strerror(res));
					return;
				}
				continue;  // ok, data should be accepted again
			}
			else if (state != SND_PCM_STATE_DRAINING)
			{
				cd_warning ("read/write error, state = %s", snd_pcm_state_name(state));
				return;
			}
		}
		else if (r == -ESTRPIPE)  // a suspend event occurred (stream is suspended and waiting for an application recovery)
		{
			cd_debug ("suspend");
			int res;
			while ((res = snd_pcm_resume (handle)) == -EAGAIN)
				sleep(1);	// wait until suspend flag is released
			if (res < 0)  // failed, restart stream
			{
				if ((res = snd_pcm_prepare(handle)) < 0)
				{
					cd_warning ("suspend: prepare error: %s", snd_strerror(res));
					return ;
				}
			}
		}
		else if (r < 0)
		{
			cd_warning ("write error: %s", snd_strerror(r));
			return ;
		}
		if (r > 0)
		{
			n -= r;
			buffer += r;
		}
	}
	
	pSharedMemory->handle = handle;
	snd_pcm_drain (handle);  // flush data
}

void cd_sound_play_sound (CDSoundFile *pSoundFile)
{
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->pSoundFile = pSoundFile;  // the sound file is loaded before, and will stay alive until the task is stopped.
	CairoDockTask *pTask = cairo_dock_new_task_full (0,  // 1 shot task.
		(CairoDockGetDataAsyncFunc) _play_sound_async,
		(CairoDockUpdateSyncFunc) _finish_play_sound,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	pSharedMemory->pTask = pTask;
	myData.pTasks = g_list_prepend (myData.pTasks, pTask);
	
	cairo_dock_launch_task (pTask);
}


void cd_sound_free_sound_file (CDSoundFile *pSoundFile)
{
	if (!pSoundFile)
		return;
	g_free (pSoundFile->buffer);  // 'data' points inside 'buffer'.
	g_free (pSoundFile);
	
}

void cd_sound_free_current_tasks (void)
{
	CairoDockTask *pTask;
	GList *t = myData.pTasks, *next_t;
	while (t != NULL)
	{
		next_t = t->next;
		pTask = t->data;
		cairo_dock_free_task (pTask);  // will remove it from the list.
		t = next_t;
	}  // at this point the list is empty and myData.pTasks is NULL.
}
