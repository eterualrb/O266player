/*****************************************************************************
 * input_dec.c: Functions for the management of decoders
 *****************************************************************************
 * Copyright (C) 1999-2001 VideoLAN
 * $Id: input_dec.c,v 1.40 2002/07/23 00:39:17 sam Exp $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include <stdlib.h>
#include <string.h>                                    /* memcpy(), memset() */
#include <sys/types.h>                                              /* off_t */

#include <vlc/vlc.h>

#include "stream_control.h"
#include "input_ext-dec.h"
#include "input_ext-intf.h"
#include "input_ext-plugins.h"

static decoder_fifo_t * CreateDecoderFifo( input_thread_t *,
                                           es_descriptor_t * );
static void             DeleteDecoderFifo( decoder_fifo_t * );

/*****************************************************************************
 * input_RunDecoder: spawns a new decoder thread
 *****************************************************************************/
decoder_fifo_t * input_RunDecoder( input_thread_t * p_input,
                                   es_descriptor_t * p_es )
{
    char * psz_plugin = config_GetPsz( p_input, "codec" );

    /* Get a suitable module */
    p_es->p_module = module_Need( p_input, MODULE_CAPABILITY_DECODER,
                                  psz_plugin, (void *)&p_es->i_fourcc );
    if( psz_plugin ) free( psz_plugin );
    if( p_es->p_module == NULL )
    {
        msg_Err( p_input, "no suitable decoder module for fourcc `%4.4s'",
                          (char*)&p_es->i_fourcc );
        return NULL;
    }

    /* Create the decoder configuration structure */
    p_es->p_decoder_fifo = CreateDecoderFifo( p_input, p_es );

    if( p_es->p_decoder_fifo == NULL )
    {
        msg_Err( p_input, "could not create decoder fifo" );
        module_Unneed( p_es->p_module );
        return NULL;
    }

    /* Spawn the decoder thread */
    if ( vlc_thread_create( p_es->p_decoder_fifo, "decoder",
                 p_es->p_module->p_functions->dec.functions.dec.pf_run, 0 ) )
    {
        msg_Err( p_input, "cannot spawn decoder thread \"%s\"",
                           p_es->p_module->psz_object_name );
        DeleteDecoderFifo( p_es->p_decoder_fifo );
        module_Unneed( p_es->p_module );
        return NULL;
    }

    p_input->stream.b_changed = 1;

    return p_es->p_decoder_fifo;
}


/*****************************************************************************
 * input_EndDecoder: kills a decoder thread and waits until it's finished
 *****************************************************************************/
void input_EndDecoder( input_thread_t * p_input, es_descriptor_t * p_es )
{
    int i_dummy;

    p_es->p_decoder_fifo->b_die = 1;

    /* Make sure the thread leaves the NextDataPacket() function by
     * sending it a few null packets. */
    for( i_dummy = 0; i_dummy < PADDING_PACKET_NUMBER; i_dummy++ )
    {
        input_NullPacket( p_input, p_es );
    }

    if( p_es->p_pes != NULL )
    {
        input_DecodePES( p_es->p_decoder_fifo, p_es->p_pes );
    }

    /* Waiting for the thread to exit */
    /* I thought that unlocking was better since thread join can be long
     * but it actually creates late pictures and freezes --stef */
//    vlc_mutex_unlock( &p_input->stream.stream_lock );
    vlc_thread_join( p_es->p_decoder_fifo );
//    vlc_mutex_lock( &p_input->stream.stream_lock );

    /* Delete decoder configuration */
    DeleteDecoderFifo( p_es->p_decoder_fifo );

    /* Unneed module */
    module_Unneed( p_es->p_module );

    /* Tell the input there is no more decoder */
    p_es->p_decoder_fifo = NULL;

    p_input->stream.b_changed = 1;
}

/*****************************************************************************
 * input_DecodePES
 *****************************************************************************
 * Put a PES in the decoder's fifo.
 *****************************************************************************/
void input_DecodePES( decoder_fifo_t * p_decoder_fifo, pes_packet_t * p_pes )
{
    vlc_mutex_lock( &p_decoder_fifo->data_lock );

    p_pes->p_next = NULL;
    *p_decoder_fifo->pp_last = p_pes;
    p_decoder_fifo->pp_last = &p_pes->p_next;
    p_decoder_fifo->i_depth++;

    /* Warn the decoder that it's got work to do. */
    vlc_cond_signal( &p_decoder_fifo->data_wait );
    vlc_mutex_unlock( &p_decoder_fifo->data_lock );
}

/*****************************************************************************
 * input_EscapeDiscontinuity: send a NULL packet to the decoders
 *****************************************************************************/
void input_EscapeDiscontinuity( input_thread_t * p_input )
{
    int     i_es, i;

    for( i_es = 0; i_es < p_input->stream.i_selected_es_number; i_es++ )
    {
        es_descriptor_t * p_es = p_input->stream.pp_selected_es[i_es];

        if( p_es->p_decoder_fifo != NULL )
        {
            for( i = 0; i < PADDING_PACKET_NUMBER; i++ )
            {
                input_NullPacket( p_input, p_es );
            }
        }
    }
}

/*****************************************************************************
 * input_EscapeAudioDiscontinuity: send a NULL packet to the audio decoders
 *****************************************************************************/
void input_EscapeAudioDiscontinuity( input_thread_t * p_input )
{
    int     i_es, i;

    for( i_es = 0; i_es < p_input->stream.i_selected_es_number; i_es++ )
    {
        es_descriptor_t * p_es = p_input->stream.pp_selected_es[i_es];

        if( p_es->p_decoder_fifo != NULL && p_es->i_cat == AUDIO_ES )
        {
            for( i = 0; i < PADDING_PACKET_NUMBER; i++ )
            {
                input_NullPacket( p_input, p_es );
            }
        }
    }
}

/*****************************************************************************
 * CreateDecoderFifo: create a decoder_fifo_t
 *****************************************************************************/
static decoder_fifo_t * CreateDecoderFifo( input_thread_t * p_input,
                                           es_descriptor_t * p_es )
{
    decoder_fifo_t * p_fifo;

    /* Decoder FIFO */
    p_fifo = vlc_object_create( p_input, VLC_OBJECT_DECODER );
    if( p_fifo == NULL )
    {
        msg_Err( p_input, "out of memory" );
        return NULL;
    }

    /* Select a new ES */
    p_input->stream.i_selected_es_number++;
    p_input->stream.pp_selected_es = realloc(
                                          p_input->stream.pp_selected_es,
                                          p_input->stream.i_selected_es_number
                                           * sizeof(es_descriptor_t *) );
    if( p_input->stream.pp_selected_es == NULL )
    {
        msg_Err( p_input, "out of memory" );
        vlc_object_destroy( p_fifo );
        return NULL;
    }

    p_input->stream.pp_selected_es[p_input->stream.i_selected_es_number - 1]
            = p_es;

    /* Initialize the p_fifo structure */
    vlc_mutex_init( p_input, &p_fifo->data_lock );
    vlc_cond_init( p_input, &p_fifo->data_wait );
    p_es->p_decoder_fifo = p_fifo;

    p_fifo->i_id = p_es->i_id;
    p_fifo->i_fourcc = p_es->i_fourcc;
    p_fifo->p_demux_data = p_es->p_demux_data;
    
    p_fifo->p_stream_ctrl = &p_input->stream.control;

    p_fifo->p_first = NULL;
    p_fifo->pp_last = &p_fifo->p_first;
    p_fifo->i_depth = 0;
    p_fifo->b_die = p_fifo->b_error = 0;
    p_fifo->p_packets_mgt = p_input->p_method_data;

    vlc_object_attach( p_fifo, p_input );

    return p_fifo;
}

/*****************************************************************************
 * DeleteDecoderFifo: destroy a decoder_fifo_t
 *****************************************************************************/
static void DeleteDecoderFifo( decoder_fifo_t * p_fifo )
{
    vlc_object_detach_all( p_fifo );

    msg_Dbg( p_fifo, "killing decoder for 0x%x, fourcc `%4.4s', %d PES in FIFO",
                     p_fifo->i_id, (char*)&p_fifo->i_fourcc, p_fifo->i_depth );

    /* Free all packets still in the decoder fifo. */
    input_DeletePES( p_fifo->p_packets_mgt,
                     p_fifo->p_first );

    /* Destroy the lock and cond */
    vlc_cond_destroy( &p_fifo->data_wait );
    vlc_mutex_destroy( &p_fifo->data_lock );

    vlc_object_destroy( p_fifo );
}

