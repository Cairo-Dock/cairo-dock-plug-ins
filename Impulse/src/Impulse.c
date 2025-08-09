/*
 *
 *+  Copyright (c) 2009 Ian Halpern
 *@  http://impulse.ian-halpern.com
 *+  Copyright (c) 2011-2012 Matthieu Baerts (Cairo-Dock Project)
 *@  http://glx-dock.org
 *
 *   This file is part of Impulse.
 *
 *   Impulse is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Impulse is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Impulse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pulse/pulseaudio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#ifdef FFT_IS_AVAILABLE
#include <fftw3.h>
#endif
#include <math.h>
#include <glib.h>

#include "Impulse.h"
#include <cairo-dock-log.h>

#define CHUNK 1024

#ifdef FFT_IS_AVAILABLE
static const long s_fft_max[] = { 12317168L, 7693595L, 5863615L, 4082974L, 5836037L, 4550263L, 3377914L, 3085778L, 3636534L, 3751823L, 2660548L, 3313252L, 2698853L, 2186441L, 1697466L, 1960070L, 1286950L, 1252382L, 1313726L, 1140443L, 1345589L, 1269153L, 897605L, 900408L, 892528L, 587972L, 662925L, 668177L, 686784L, 656330L, 1580286L, 785491L, 761213L, 730185L, 851753L, 927848L, 891221L, 634291L, 833909L, 646617L, 804409L, 1015627L, 671714L, 813811L, 689614L, 727079L, 853936L, 819333L, 679111L, 730295L, 836287L, 1602396L, 990827L, 773609L, 733606L, 638993L, 604530L, 573002L, 634570L, 1015040L, 679452L, 672091L, 880370L, 1140558L, 1593324L, 686787L, 781368L, 605261L, 1190262L, 525205L, 393080L, 409546L, 436431L, 723744L, 765299L, 393927L, 322105L, 478074L, 458596L, 512763L, 381303L, 671156L, 1177206L, 476813L, 366285L, 436008L, 361763L, 252316L, 204433L, 291331L, 296950L, 329226L, 319209L, 258334L, 388701L, 543025L, 396709L, 296099L, 190213L, 167976L, 138928L, 116720L, 163538L, 331761L, 133932L, 187456L, 530630L, 131474L, 84888L, 82081L, 122379L, 82914L, 75510L, 62669L, 73492L, 68775L, 57121L, 94098L, 68262L, 68307L, 48801L, 46864L, 61480L, 46607L, 45974L, 45819L, 45306L, 45110L, 45175L, 44969L, 44615L, 44440L, 44066L, 43600L, 57117L, 43332L, 59980L, 55319L, 54385L, 81768L, 51165L, 54785L, 73248L, 52494L, 57252L, 61869L, 65900L, 75893L, 65152L, 108009L, 421578L, 152611L, 135307L, 254745L, 132834L, 169101L, 137571L, 141159L, 142151L, 211389L, 267869L, 367730L, 256726L, 185238L, 251197L, 204304L, 284443L, 258223L, 158730L, 228565L, 375950L, 294535L, 288708L, 351054L, 694353L, 477275L, 270576L, 426544L, 362456L, 441219L, 313264L, 300050L, 421051L, 414769L, 244296L, 292822L, 262203L, 418025L, 579471L, 418584L, 419449L, 405345L, 739170L, 488163L, 376361L, 339649L, 313814L, 430849L, 275287L, 382918L, 297214L, 286238L, 367684L, 303578L, 516246L, 654782L, 353370L, 417745L, 392892L, 418934L, 475608L, 284765L, 260639L, 288961L, 301438L, 301305L, 329190L, 252484L, 272364L, 261562L, 208419L, 203045L, 229716L, 191240L, 328251L, 267655L, 322116L, 509542L, 498288L, 341654L, 346341L, 451042L, 452194L, 467716L, 447635L, 644331L, 1231811L, 1181923L, 1043922L, 681166L, 1078456L, 1088757L, 1221378L, 1358397L, 1817252L, 1255182L, 1410357L, 2264454L, 1880361L, 1630934L, 1147988L, 1919954L, 1624734L, 1373554L, 1865118L, 2431931L };
#else
#define MAXVALUE 32768 // 2^15
#endif

static uint32_t source_index = 0;
static int use_sink = 0;
static int use_monitor = 0;
// we allocate 3 buffers to avoid reading and writing to the same
static int16_t buffer1[ CHUNK / 2 ], buffer2[ CHUNK / 2 ], buffer3[ CHUNK / 2 ];
static int16_t *buffer_ready = buffer1; // buffer that is currently full and can be used in im_getSnapshot ()
static int16_t *buffer_in_snapshot = buffer2; // buffer that is currently used in im_getSnapshot ()
static int16_t *buffer_update = buffer3; // buffer that is being filled by stream_read_callback ()
static int have_buffer_update = 0; // whether there is new data in buffer_ready
static size_t buffer_index = 0;
static int stream_running = 0;
static int failed = 0;
static int initial_setup = 0;

static pa_context *context = NULL;
static pa_stream *stream = NULL;
static pa_threaded_mainloop* mainloop = NULL;
static pa_io_event* stdio_event = NULL;
static pa_mainloop_api *mainloop_api = NULL;

static pa_sample_spec sample_spec = {
	.format = PA_SAMPLE_S16LE,
	.rate = 44100,
	.channels = 2
};

#if GLIB_CHECK_VERSION (2, 74, 0)
#define _atomic_pointer_exchange g_atomic_pointer_exchange
#else
static inline int16_t *_atomic_pointer_exchange (int16_t **atomic, int16_t *newval)
{
	// older glib does not have exchange, emulate it with compare_and_exchange
	int16_t *oldval;
	do oldval = g_atomic_pointer_get (atomic);
	while (! g_atomic_pointer_compare_and_exchange (atomic, oldval, newval));
	return oldval;
}
#endif


static pa_stream_flags_t flags = 0;

static pa_channel_map channel_map;
static int channel_map_set = 0;

/* A shortcut for terminating the application */
static void quit( int ret ) {
	stream_running = 0;
	if (mainloop_api) mainloop_api->quit( mainloop_api, ret );
	failed = 1;
}

static void get_source_info_callback( pa_context *c, const pa_source_info *i, int is_last, void *userdata ) {

	if ( !i ) return;
	cd_debug ( "%s", i->name );

	if ( ( pa_stream_connect_record( stream, i->name, NULL, flags ) ) < 0 ) {
		cd_warning ("pa_stream_connect_record() failed: %s", pa_strerror(pa_context_errno(c)));
		quit(1);
	}
}

static void get_sink_info_callback( pa_context *c, const pa_sink_info *i, int is_last, void *userdata ) {

	if ( !i ) return;
	cd_debug ( "%s -- %s", i->name, i->monitor_source_name );

	if ( ( pa_stream_connect_record( stream,
		(use_monitor && i->monitor_source_name) ? i->monitor_source_name : i->name,
		NULL, flags ) ) < 0 )
	{
		cd_warning ("pa_stream_connect_record() failed: %s", pa_strerror(pa_context_errno(c)));
		quit(1);
	}
}

/* This is called whenever new data is available */
static void stream_read_callback(pa_stream *s, size_t length, void *userdata) {
	const void *data;
	assert(s);
	assert(length > 0);
//	printf("stream index: %d\n", pa_stream_get_index( s ) );
	if (stdio_event)
		mainloop_api->io_enable(stdio_event, PA_IO_EVENT_OUTPUT);

	if (pa_stream_peek(s, &data, &length) < 0) {
		fprintf(stderr, "pa_stream_peek() failed: %s\n", pa_strerror(pa_context_errno(context)));
		quit(1);
		return;
	}

	assert(data);
	assert(length > 0);

	int excess = buffer_index * 2 + length - ( CHUNK );

	if ( excess < 0 ) excess = 0;

	memcpy( buffer_update + buffer_index, data, length - excess );
	buffer_index += ( length - excess ) / 2;

	if ( excess ) {
		buffer_update = _atomic_pointer_exchange (&buffer_ready, buffer_update);
		g_atomic_int_set (&have_buffer_update, 1);
		buffer_index = 0;
	}

	pa_stream_drop(s);
}

static void stream_state_callback( pa_stream *s, void* userdata );

static void init_source_stream_for_recording(void) {
	if (! stream)
	{
		if (!(stream = pa_stream_new( context, "impulse", &sample_spec, channel_map_set ? &channel_map : NULL))) {
			fprintf(stderr, "pa_stream_new() failed: %s\n", pa_strerror(pa_context_errno(context)));
			quit(1);
		}

		pa_stream_set_read_callback(stream, stream_read_callback, NULL);
		pa_stream_set_state_callback( stream, stream_state_callback, NULL );
	}
	if (use_sink) pa_operation_unref( pa_context_get_sink_info_by_index( context, source_index, get_sink_info_callback, NULL ) );
	else pa_operation_unref( pa_context_get_source_info_by_index( context, source_index, get_source_info_callback, NULL ) );
}

static void stream_state_callback( pa_stream *s, void* userdata ) {
	if ( pa_stream_get_state( s ) == PA_STREAM_TERMINATED ) {
		pa_stream_unref( stream );
		stream = NULL;
		if (stream_running) init_source_stream_for_recording();
	}
}

static void context_state_callback( pa_context *c, void *userdata ) {

	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
		case PA_CONTEXT_READY:
			assert(c);
			assert(!stream);

			/*if (!(stream = pa_stream_new(c, stream_name, &sample_spec, channel_map_set ? &channel_map : NULL))) {
				fprintf(stderr, "pa_stream_new() failed: %s\n", pa_strerror(pa_context_errno(c)));
				quit(1);
			}

			pa_stream_set_read_callback(stream, stream_read_callback, NULL);*/
			if (initial_setup)
			{
				init_source_stream_for_recording();
				initial_setup = 0;
			}

			break;
		case PA_CONTEXT_TERMINATED:
			quit(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
			quit(1);
	}
}

int im_context_state (void)
{
	if (context == NULL)
	return IM_FAILED;

	switch (pa_context_get_state (context))
	{
		case PA_CONTEXT_TERMINATED:
		case PA_CONTEXT_FAILED:
			return IM_FAILED;
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
		default: // default seems to be ok...
			return IM_SUCCESS;
	}
}

void im_stop (void) {
	stream_running = 0;
	if (stream && pa_stream_get_state( stream ) != PA_STREAM_UNCONNECTED)
		pa_stream_disconnect( stream );

	cd_debug ( "exit" );
}

void im_setSourceProperties( uint32_t index, int bUseSink, int bUseMonitor ) {
	source_index = index;
	use_sink = bUseSink;
	use_monitor = bUseMonitor;
	if ( !stream ) return;

	if ( pa_stream_get_state( stream ) != PA_STREAM_UNCONNECTED )
		pa_stream_disconnect( stream );
	else
		init_source_stream_for_recording();
}

double *im_getSnapshot (void)
{
	static double magnitude [CHUNK / 4];
	int i;
	
	if (!stream_running) return NULL;

	if (g_atomic_int_exchange (&have_buffer_update, 0))
	{
		buffer_in_snapshot = _atomic_pointer_exchange (&buffer_update, buffer_in_snapshot);
	}

#ifdef FFT_IS_AVAILABLE
	static double in [CHUNK / 2];
	static fftw_complex out [CHUNK / 2]; // should be enough to have CHUNK / 4 + 1 ?
	fftw_plan p;

	for (i = 0; i < CHUNK / 2; i++)
		in[ i ] = (double) buffer_in_snapshot[ i ];

	p = fftw_plan_dft_r2c_1d( CHUNK / 2, in, out, 0 );

	fftw_execute( p );

	fftw_destroy_plan( p );

	for (i = 0; i < CHUNK / 2 / sample_spec.channels; i++)
	{
		magnitude[ i ] = (double) sqrt( pow( out[ i ][ 0 ], 2 ) + pow( out[ i ][ 1 ], 2 ) ) / s_fft_max[ i ];
		if (magnitude[ i ] > 1.0 )
			magnitude[ i ] = 1.0;
	}

#else
	int iSnapshot, iCurrentMagnitude;
	for (i = 0; i < CHUNK / 2; i += sample_spec.channels)
	{
		int j;
		iCurrentMagnitude = i / sample_spec.channels; // 1 => 256 (= CHUNK / 2 / channels)
		magnitude [iCurrentMagnitude] = 0; // init
		for (j = 0; j < sample_spec.channels; j++)
		{
			iSnapshot = buffer_in_snapshot [i + j];
			if (iSnapshot > 0)
				magnitude [iCurrentMagnitude] += (double) iSnapshot / MAXVALUE;
		}
		if (magnitude [iCurrentMagnitude] < 10e-5 && iCurrentMagnitude != 0)
			magnitude [iCurrentMagnitude] = magnitude [iCurrentMagnitude - 1]; // strange to have 0...
		magnitude [iCurrentMagnitude] = magnitude [iCurrentMagnitude] / sample_spec.channels / 1.75; // mean value / 1.66 => 
	}
#endif

	return magnitude;
}


void im_start ( void ) {
	if (failed) return; // we already had an error before

	// Pulseaudio
	int r, i;
	char *server = NULL;

	for (i = 0; i < CHUNK / 2; i++) buffer_in_snapshot[i] = 0;

	if (! mainloop)
	{
		// First time setup
		initial_setup = 1;
		
		// Set up a new main loop
		mainloop = pa_threaded_mainloop_new();
		if (! mainloop)
		{
			fprintf( stderr, "pa_mainloop_new() failed.\n" );
			return; // IM_FAILED;
		}
		mainloop_api = pa_threaded_mainloop_get_api( mainloop );
		r = pa_signal_init( mainloop_api );
		if (r) return;
		
		// create a new connection context
		context = pa_context_new( mainloop_api, "impulse" );
		if (! context)
		{
			cd_warning ("pa_context_new() failed.");
			return; // IM_FAILED;
		}
		
		pa_context_set_state_callback( context, context_state_callback, NULL );

		/* Connect the context */
		pa_context_connect( context, server, 0, NULL );
		
		// pulseaudio thread
		pa_threaded_mainloop_start( mainloop );
	}
	else init_source_stream_for_recording();

	stream_running = 1;
	return; // context_state (context);
}
