/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "patata_app.h"
#include "patata_main.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

#include "hardware/wifictl.h"

#include "utils/json_psram_allocator.h"
#include "utils/alloc.h"

#ifdef NATIVE_64BIT
    #include "utils/logging.h"
    #include "utils/millis.h"
    #include <mosquitto.h>
    
    struct mosquitto *mosq;
#else
    #include <Arduino.h>
    #include <WiFi.h>
    #include <PubSubClient.h>

#endif

lv_obj_t *patata_main_tile = NULL;
lv_style_t patata_main_style;
lv_style_t patata_id_style;

lv_task_t * _patata_main_task;

lv_obj_t *id_cont2 = NULL;
lv_obj_t *id_label2 = NULL;
lv_obj_t *voltage_cont2 = NULL;
lv_obj_t *voltage_label2 = NULL;
lv_obj_t *current_cont2 = NULL;
lv_obj_t *current_label2 = NULL;
lv_obj_t *power_cont2 = NULL;
lv_obj_t *power_label2 = NULL;

LV_IMG_DECLARE(refresh_32px);
LV_FONT_DECLARE(Ubuntu_16px);
LV_FONT_DECLARE(Ubuntu_48px);

bool patata_style_change_event_cb( EventBits_t event, void *arg );
bool patata_wifictl_event_cb( EventBits_t event, void *arg );
static void enter_patata_setup_event_cb( lv_obj_t * obj, lv_event_t event );
void patata_main_task( lv_task_t * task );

#ifdef NATIVE_64BIT
    void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
        patata_config_t *patata_config = patata_get_config();
        int rc;
        log_i("on_connect: %s\n", mosquitto_connack_string( reason_code ) );
        if(reason_code != 0){
            mosquitto_disconnect(mosq);
        }
        log_i("subscripe: %s", patata_config->topic );
        rc = mosquitto_subscribe(mosq, NULL, patata_config->topic, 1);
        if(rc != MOSQ_ERR_SUCCESS){
            log_i( "Error subscribing: %s\n", mosquitto_strerror( rc ) );
            mosquitto_disconnect(mosq);
        }
    }

    void callback (struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)  {
        unsigned int length = msg->payloadlen;
        char *payload = (char *)msg->payload;
#else
   // void callback(char* topic, byte* payload, unsigned int length) {
#endif
    /**
     * alloc a msg buffer and copy payload and terminate it with '\0';
     */
/*     char *mqttmsg = (char*)CALLOC_ASSERT( length + 1, 1, "mqttmsg calloc failed" );
    memcpy( mqttmsg, payload, length );

    SpiRamJsonDocument doc( strlen( mqttmsg ) * 2 );
    DeserializationError error = deserializeJson( doc, mqttmsg );

    if ( error ) {
        log_e("patata message deserializeJson() failed: %s", error.c_str() );
    }
    else  {
        if ( doc.containsKey("id") ) {
            lv_label_set_text( id_label2, doc["id"] );
        }
        if ( doc["all"].containsKey("power") ) {
            const char * unit = "kW";
            if( doc.containsKey("PowerUnit") )
                unit = doc["PowerUnit"];
            wf_label_printf( power_label2, "%0.2fkW", atof( doc["all"]["power"] ), unit );
        }
        if ( doc["channel0"].containsKey("power") ) {
            const char * unit = "kW";
            if( doc.containsKey("PowerUnit") )
                unit = doc["PowerUnit"];
            wf_label_printf( power_label2, "%0.2f%s", atof( doc["channel0"]["power"] ), unit );
        }
        if ( doc["channel0"].containsKey("voltage") ) {
            const char * unit = "V";
            if( doc.containsKey("VoltageUnit") )
                unit = doc["VoltageUnit"];
            wf_label_printf( voltage_label2, "%0.1f%s", atof( doc["channel0"]["voltage"] ), unit );
        }
        if ( doc["channel0"].containsKey("current") ) {
            const char * unit = "A";
            if( doc.containsKey("CurrentUnit") )
                unit = doc["CurrentUnit"];
            wf_label_printf( current_label2, "%0.1f%s", atof( doc["channel0"]["current"] ), unit );
        }

        lv_obj_align( id_label2, id_cont2, LV_ALIGN_IN_RIGHT_MID, -THEME_PADDING, 0 );
        lv_obj_align( power_label2, power_cont2, LV_ALIGN_IN_RIGHT_MID, -THEME_PADDING, 0 );
        lv_obj_align( voltage_label2, voltage_cont2, LV_ALIGN_IN_RIGHT_MID, -THEME_PADDING, 0 );
        lv_obj_align( current_label2, current_cont2, LV_ALIGN_IN_RIGHT_MID, -THEME_PADDING, 0 );
    }

    doc.clear();
    free( mqttmsg );
} */

void patata_main_tile_setup( uint32_t tile_num ) {

    patata_main_tile = mainbar_get_tile_obj( tile_num );

    lv_style_copy( &patata_main_style, APP_STYLE );
    lv_style_set_text_font( &patata_main_style, LV_STATE_DEFAULT, &Ubuntu_48px);
    lv_obj_add_style( patata_main_tile, LV_OBJ_PART_MAIN, &patata_main_style );

    lv_style_copy( &patata_id_style, APP_STYLE );
    lv_style_set_text_font( &patata_id_style, LV_STATE_DEFAULT, &Ubuntu_16px);

    lv_obj_t * exit_btn = wf_add_exit_button( patata_main_tile, SYSTEM_ICON_STYLE );
    lv_obj_align(exit_btn, patata_main_tile, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10 );

    lv_obj_t * setup_btn = wf_add_setup_button( patata_main_tile, enter_patata_setup_event_cb, SYSTEM_ICON_STYLE );
    lv_obj_align(setup_btn, patata_main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10 );

    id_cont2 = lv_obj_create( patata_main_tile, NULL );
    lv_obj_set_size( id_cont2, lv_disp_get_hor_res( NULL ), 20 );
    lv_obj_add_style( id_cont2, LV_OBJ_PART_MAIN, &patata_id_style );
    lv_obj_align( id_cont2, patata_main_tile, LV_ALIGN_IN_TOP_MID, 0, 0 );
    lv_obj_t * id_info_label = lv_label_create( id_cont2, NULL );
    lv_obj_add_style( id_info_label, LV_OBJ_PART_MAIN, &patata_id_style );
    lv_label_set_text( id_info_label, "ID:" );
    lv_obj_align( id_info_label, id_cont2, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    id_label2 = lv_label_create( id_cont2, NULL );
    lv_obj_add_style( id_label2, LV_OBJ_PART_MAIN, &patata_id_style );
    lv_label_set_text( id_label2, "n/a" );
    lv_obj_align( id_label2, id_cont2, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    voltage_cont2 = lv_obj_create( patata_main_tile, NULL );
    lv_obj_set_size( voltage_cont2, lv_disp_get_hor_res( NULL ), 56 );
    lv_obj_add_style( voltage_cont2, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_obj_align( voltage_cont2, id_cont2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * voltage_info_label = lv_label_create( voltage_cont2, NULL );
    lv_obj_add_style( voltage_info_label, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_label_set_text( voltage_info_label, "U =" );
    lv_obj_align( voltage_info_label, voltage_cont2, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    voltage_label2 = lv_label_create( voltage_cont2, NULL );
    lv_obj_add_style( voltage_label2, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_label_set_text( voltage_label2, "n/a" );
    lv_obj_align( voltage_label2, voltage_cont2, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    current_cont2 = lv_obj_create( patata_main_tile, NULL );
    lv_obj_set_size( current_cont2, lv_disp_get_hor_res( NULL ), 56 );
    lv_obj_add_style( current_cont2, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_obj_align( current_cont2, voltage_cont2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * current_info_label = lv_label_create( current_cont2, NULL );
    lv_obj_add_style( current_info_label, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_label_set_text( current_info_label, "I =" );
    lv_obj_align( current_info_label, current_cont2, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    current_label2 = lv_label_create( current_cont2, NULL );
    lv_obj_add_style( current_label2, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_label_set_text( current_label2, "n/a" );
    lv_obj_align( current_label2, current_cont2, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

    power_cont2 = lv_obj_create( patata_main_tile, NULL );
    lv_obj_set_size( power_cont2, lv_disp_get_hor_res( NULL ), 56 );
    lv_obj_add_style( power_cont2, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_obj_align( power_cont2, current_cont2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t * power_info_label = lv_label_create( power_cont2, NULL );
    lv_obj_add_style( power_info_label, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_label_set_text( power_info_label, "P =" );
    lv_obj_align( power_info_label, power_cont2, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    power_label2 = lv_label_create( power_cont2, NULL );
    lv_obj_add_style( power_label2, LV_OBJ_PART_MAIN, &patata_main_style );
    lv_label_set_text( power_label2, "n/a" );
    lv_obj_align( power_label2, power_cont2, LV_ALIGN_IN_RIGHT_MID, -5, 0 );

#ifdef NATIVE_64BIT
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    mosquitto_message_callback_set(mosq, callback );
    mosquitto_connect_callback_set(mosq, on_connect);
#else
  /*   patata_mqtt_client.setCallback( callback );
    patata_mqtt_client.setBufferSize( 512 ); */
#endif
    wifictl_register_cb( WIFICTL_CONNECT_IP | WIFICTL_OFF_REQUEST | WIFICTL_OFF | WIFICTL_DISCONNECT , patata_wifictl_event_cb, "patata" );
    styles_register_cb( STYLE_CHANGE, patata_style_change_event_cb, "patata style event ");
    // create an task that runs every secound
    _patata_main_task = lv_task_create( patata_main_task, 250, LV_TASK_PRIO_MID, NULL );
}

bool patata_style_change_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case STYLE_CHANGE:  lv_style_copy( &patata_main_style, APP_STYLE );
                            lv_style_set_text_font( &patata_main_style, LV_STATE_DEFAULT, &Ubuntu_48px);
                            lv_style_copy( &patata_id_style, APP_STYLE );
                            lv_style_set_text_font( &patata_id_style, LV_STATE_DEFAULT, &Ubuntu_16px);
                            break;
        case STYLE_DARKMODE:
                            break;
        case STYLE_LIGHTMODE:
                            break;
    }
    return( true );
}

bool patata_wifictl_event_cb( EventBits_t event, void *arg ) {
//    patata_config_t *patata_config = patata_get_config();

    switch( event ) {
        case WIFICTL_CONNECT_IP:    
#ifdef NATIVE_64BIT
                                    if ( patata_config->autoconnect ) {
                                        mosquitto_username_pw_set( mosq, patata_config->user, patata_config->password );
                                        int rc = mosquitto_connect( mosq, patata_config->server, patata_config->port, 60 );
                                        log_i("connect to : %s", patata_config->server );
                                      	if(rc != MOSQ_ERR_SUCCESS){
                                            mosquitto_destroy(mosq);
                                            log_i("Error: %s\n", mosquitto_strerror(rc) );
                                            return 1;
                                    	}
                                    }
#else
                                    /* if ( patata_config->autoconnect ) {
                                        patata_mqtt_client.setServer( patata_config->server, patata_config->port );
                                        if ( !patata_mqtt_client.connect( "patata", patata_config->user, patata_config->password ) ) {
                                            log_e("connect to mqtt server %s failed", patata_config->server );
                                            app_set_indicator( patata_get_app_icon(), ICON_INDICATOR_FAIL );
                                            widget_set_indicator( patata_get_widget_icon() , ICON_INDICATOR_FAIL );
                                        }
                                        else {
                                            log_i("connect to mqtt server %s success", patata_config->server );
                                            patata_mqtt_client.subscribe( patata_config->topic );
                                            app_set_indicator( patata_get_app_icon(), ICON_INDICATOR_OK );
                                            widget_set_indicator( patata_get_widget_icon(), ICON_INDICATOR_OK );
                                        }
                                    } */
#endif                               
                                    break;
        case WIFICTL_OFF_REQUEST:
        case WIFICTL_OFF:
        case WIFICTL_DISCONNECT:    
#ifdef NATIVE_64BIT
                                    mosquitto_disconnect(mosq);
#else
                                    /* if ( patata_mqtt_client.connected() ) {
                                        log_i("disconnect from mqtt server %s", patata_config->server );
                                        patata_mqtt_client.disconnect();
                                        app_hide_indicator( patata_get_app_icon() );
                                        widget_hide_indicator( patata_get_widget_icon() );
                                        widget_set_label( patata_get_widget_icon(), "n/a" );
                                    } */
#endif
                                    break;
    }
    return( true );
}

static void enter_patata_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( patata_get_app_setup_tile_num(), LV_ANIM_ON, true );
                                        break;
    }
}

void patata_main_task( lv_task_t * task ) {
    // put your code her
#ifdef NATIVE_64BIT
    mosquitto_loop( mosq, 60, 10 );
#else
   // patata_mqtt_client.loop();
#endif
}