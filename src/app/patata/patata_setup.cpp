/****************************************************************************
 *   Tu May 22 21:23:51 2020
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
#include <config.h>

#include "patata_app.h"
#include "patata_setup.h"
#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/statusbar.h"
#include "gui/keyboard.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

#ifdef NATIVE_64BIT
    #include "utils/logging.h"
    #include "utils/millis.h"
    #include <string>

    using namespace std;
    #define String string
#else
    #include <Arduino.h>
#endif

lv_obj_t *patata_setup_tile = NULL;
lv_obj_t *patata_setup_tile_2 = NULL;
uint32_t patata_setup_tile_num;
//------------------------------------------------------OUT
lv_obj_t *patata_server_textfield = NULL;
lv_obj_t *patata_user_textfield = NULL;
lv_obj_t *patata_password_textfield = NULL;
lv_obj_t *patata_topic_textfield = NULL;
lv_obj_t *patata_port_textfield = NULL;
lv_obj_t *patata_autoconnect_onoff = NULL;
lv_obj_t *patata_widget_onoff = NULL;

static void patata_textarea_event_cb( lv_obj_t * obj, lv_event_t event );
static void patata_num_textarea_event_cb( lv_obj_t * obj, lv_event_t event );
static void patata_autoconnect_onoff_event_handler( lv_obj_t * obj, lv_event_t event );
static void patata_widget_onoff_event_handler( lv_obj_t *obj, lv_event_t event );
static void patata_setup_page_2_event_cb( lv_obj_t *obj, lv_event_t event );
static void patata_setup_hibernate_callback ( void );


//CONFIGURATION PAGE (CALLED SETUP IN APP)
void patata_setup_tile_setup( uint32_t tile_num ) {


    mainbar_add_tile_hibernate_cb( tile_num, patata_setup_hibernate_callback );

    patata_setup_tile_num = tile_num;
    patata_setup_tile = mainbar_get_tile_obj( patata_setup_tile_num );
    patata_setup_tile_2 = mainbar_get_tile_obj( patata_setup_tile_num + 1 );

    lv_obj_t *header = wf_add_settings_header( patata_setup_tile, "patata setup" );
    lv_obj_align( header, patata_setup_tile, LV_ALIGN_IN_TOP_LEFT, THEME_ICON_PADDING, THEME_ICON_PADDING );

    lv_obj_t *header_2 = wf_add_settings_header( patata_setup_tile_2, "patata setup" );
    lv_obj_align( header_2, patata_setup_tile_2, LV_ALIGN_IN_TOP_LEFT, THEME_ICON_PADDING, THEME_ICON_PADDING );

    lv_obj_t *setup_page_2 = wf_add_down_button( patata_setup_tile, patata_setup_page_2_event_cb );
    lv_obj_align( setup_page_2, patata_setup_tile, LV_ALIGN_IN_TOP_RIGHT, -THEME_ICON_PADDING, THEME_ICON_PADDING );

    lv_obj_t *patata_server_cont = lv_obj_create( patata_setup_tile, NULL );
    lv_obj_set_size( patata_server_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( patata_server_cont, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_obj_align( patata_server_cont, header, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    lv_obj_t *patata_server_label = lv_label_create( patata_server_cont, NULL);
    lv_obj_add_style( patata_server_label, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_label_set_text( patata_server_label, "server");
    lv_obj_align( patata_server_label, patata_server_cont, LV_ALIGN_IN_LEFT_MID, 0, 0 );
    patata_server_textfield = lv_textarea_create( patata_server_cont, NULL);
    //lv_textarea_set_text( patata_server_textfield, patata_config->server );
    lv_textarea_set_pwd_mode( patata_server_textfield, false);
    lv_textarea_set_one_line( patata_server_textfield, true);
    lv_textarea_set_cursor_hidden( patata_server_textfield, true);
    lv_obj_set_width( patata_server_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( patata_server_textfield, patata_server_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( patata_server_textfield, patata_textarea_event_cb );

    lv_obj_t *patata_port_cont = lv_obj_create( patata_setup_tile, NULL );
    lv_obj_set_size( patata_port_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( patata_port_cont, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_obj_align( patata_port_cont, patata_server_cont, LV_ALIGN_OUT_BOTTOM_MID, 0,  0 );
    lv_obj_t *patata_port_label = lv_label_create( patata_port_cont, NULL);
    lv_obj_add_style( patata_port_label, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_label_set_text( patata_port_label, "port");
    lv_obj_align( patata_port_label, patata_port_cont, LV_ALIGN_IN_LEFT_MID, 0, 0 );
    patata_port_textfield = lv_textarea_create( patata_port_cont, NULL);
    char buf[10];
    //sprintf(buf, "%d", patata_config->port );
    lv_textarea_set_text( patata_port_textfield, buf);
    lv_textarea_set_pwd_mode( patata_port_textfield, false);
    lv_textarea_set_one_line( patata_port_textfield, true);
    lv_textarea_set_cursor_hidden( patata_port_textfield, true);
    lv_obj_set_width( patata_port_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( patata_port_textfield, patata_port_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( patata_port_textfield, patata_num_textarea_event_cb );

    lv_obj_t *patata_user_cont = lv_obj_create( patata_setup_tile, NULL );
    lv_obj_set_size( patata_user_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( patata_user_cont, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_obj_align( patata_user_cont, patata_port_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *patata_user_label = lv_label_create( patata_user_cont, NULL);
    lv_obj_add_style( patata_user_label, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_label_set_text( patata_user_label, "user");
    lv_obj_align( patata_user_label, patata_user_cont, LV_ALIGN_IN_LEFT_MID, 0, 0 );
    patata_user_textfield = lv_textarea_create( patata_user_cont, NULL);
    //lv_textarea_set_text( patata_user_textfield, patata_config->user );
    lv_textarea_set_pwd_mode( patata_user_textfield, false);
    lv_textarea_set_one_line( patata_user_textfield, true);
    lv_textarea_set_cursor_hidden( patata_user_textfield, true);
    lv_obj_set_width( patata_user_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( patata_user_textfield, patata_user_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( patata_user_textfield, patata_textarea_event_cb );

    lv_obj_t *patata_password_cont = lv_obj_create( patata_setup_tile, NULL );
    lv_obj_set_size( patata_password_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( patata_password_cont, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_obj_align( patata_password_cont, patata_user_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *patata_password_label = lv_label_create( patata_password_cont, NULL);
    lv_obj_add_style( patata_password_label, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_label_set_text( patata_password_label, "pass");
    lv_obj_align( patata_password_label, patata_password_cont, LV_ALIGN_IN_LEFT_MID, 0, 0 );
    patata_password_textfield = lv_textarea_create( patata_password_cont, NULL);
    //lv_textarea_set_text( patata_password_textfield, patata_config->password );
    lv_textarea_set_pwd_mode( patata_password_textfield, false);
    lv_textarea_set_one_line( patata_password_textfield, true);
    lv_textarea_set_cursor_hidden( patata_password_textfield, true);
    lv_obj_set_width( patata_password_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( patata_password_textfield, patata_password_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( patata_password_textfield, patata_textarea_event_cb );

    lv_obj_t *patata_topic_cont = lv_obj_create( patata_setup_tile, NULL );
    lv_obj_set_size( patata_topic_cont, lv_disp_get_hor_res( NULL ) , 37);
    lv_obj_add_style( patata_topic_cont, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_obj_align( patata_topic_cont, patata_password_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    lv_obj_t *patata_topic_label = lv_label_create( patata_topic_cont, NULL);
    lv_obj_add_style( patata_topic_label, LV_OBJ_PART_MAIN, SETUP_STYLE  );
    lv_label_set_text( patata_topic_label, "topic");
    lv_obj_align( patata_topic_label, patata_topic_cont, LV_ALIGN_IN_LEFT_MID, 0, 0 );
    patata_topic_textfield = lv_textarea_create( patata_topic_cont, NULL);
    //lv_textarea_set_text( patata_topic_textfield, patata_config->topic );
    lv_textarea_set_pwd_mode( patata_topic_textfield, false);
    lv_textarea_set_one_line( patata_topic_textfield, true);
    lv_textarea_set_cursor_hidden( patata_topic_textfield, true);
    lv_obj_set_width( patata_topic_textfield, lv_disp_get_hor_res( NULL ) / 4 * 3  - 5 );
    lv_obj_align( patata_topic_textfield, patata_topic_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( patata_topic_textfield, patata_textarea_event_cb );

    lv_obj_t *patata_autoconnect_onoff_cont = lv_obj_create( patata_setup_tile_2, NULL);
    lv_obj_set_size( patata_autoconnect_onoff_cont, lv_disp_get_hor_res( NULL ), 32);
    lv_obj_add_style( patata_autoconnect_onoff_cont, LV_OBJ_PART_MAIN, SETUP_STYLE );
    lv_obj_align( patata_autoconnect_onoff_cont, header_2, LV_ALIGN_OUT_BOTTOM_LEFT, -THEME_ICON_PADDING, THEME_ICON_PADDING );
    patata_autoconnect_onoff = wf_add_switch( patata_autoconnect_onoff_cont, false);
    lv_obj_align( patata_autoconnect_onoff, patata_autoconnect_onoff_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    lv_obj_set_event_cb( patata_autoconnect_onoff, patata_autoconnect_onoff_event_handler );
    lv_obj_t *patata_autoconnect_label = lv_label_create(patata_autoconnect_onoff_cont, NULL);
    lv_obj_add_style( patata_autoconnect_label, LV_OBJ_PART_MAIN, SETUP_STYLE );
    lv_label_set_text( patata_autoconnect_label, "autoconnect");
    lv_obj_align( patata_autoconnect_label, patata_autoconnect_onoff_cont, LV_ALIGN_IN_LEFT_MID, 5, 0);

    lv_obj_t *patata_widget_onoff_cont = lv_obj_create( patata_setup_tile_2, NULL);
    lv_obj_set_size( patata_widget_onoff_cont, lv_disp_get_hor_res( NULL ), 32);
    lv_obj_add_style( patata_widget_onoff_cont, LV_OBJ_PART_MAIN, SETUP_STYLE );
    lv_obj_align( patata_widget_onoff_cont, patata_autoconnect_onoff_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, THEME_ICON_PADDING );
    patata_widget_onoff = wf_add_switch( patata_widget_onoff_cont, false);
    lv_obj_align( patata_widget_onoff, patata_widget_onoff_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    lv_obj_set_event_cb( patata_widget_onoff, patata_widget_onoff_event_handler );
    lv_obj_t *patata_widget_onoff_label = lv_label_create( patata_widget_onoff_cont, NULL);
    lv_obj_add_style( patata_widget_onoff_label, LV_OBJ_PART_MAIN, SETUP_STYLE );
    lv_label_set_text( patata_widget_onoff_label, "mainbar widget");
    lv_obj_align( patata_widget_onoff_label, patata_widget_onoff_cont, LV_ALIGN_IN_LEFT_MID, 5, 0);

   /*  if ( patata_config->autoconnect )
        lv_switch_on( patata_autoconnect_onoff, LV_ANIM_OFF);
    else
        lv_switch_off( patata_autoconnect_onoff, LV_ANIM_OFF);

    if ( patata_config->widget )
        lv_switch_on( patata_widget_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( patata_widget_onoff, LV_ANIM_OFF ); */

    lv_tileview_add_element( patata_setup_tile, patata_server_cont );
    lv_tileview_add_element( patata_setup_tile, patata_port_cont );
    lv_tileview_add_element( patata_setup_tile, patata_user_cont );
    lv_tileview_add_element( patata_setup_tile, patata_password_cont );
    lv_tileview_add_element( patata_setup_tile, patata_topic_cont );
}

static void patata_setup_hibernate_callback ( void ) {
    keyboard_hide();
    //patata_config_t *patata_config = patata_get_config();
    //strncpy( patata_config->server, lv_textarea_get_text( patata_server_textfield ), sizeof( patata_config->server ) );
    //strncpy( patata_config->user, lv_textarea_get_text( patata_user_textfield ), sizeof( patata_config->user ) );
    //strncpy( patata_config->password, lv_textarea_get_text( patata_password_textfield ), sizeof( patata_config->password ) );
    //strncpy( patata_config->topic, lv_textarea_get_text( patata_topic_textfield ), sizeof( patata_config->topic ) );
    //patata_config->port = atoi(lv_textarea_get_text( patata_port_textfield ));
    //patata_config->save();
}

static void patata_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        keyboard_set_textarea( obj );
    }
}

static void patata_num_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        num_keyboard_set_textarea( obj );
    }
}

static void patata_setup_page_2_event_cb( lv_obj_t *obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        mainbar_jump_to_tilenumber( patata_setup_tile_num + 1, LV_ANIM_OFF );
    }
    
}
static void patata_autoconnect_onoff_event_handler( lv_obj_t * obj, lv_event_t event ) {
 /*    switch ( event ) {
        case (LV_EVENT_VALUE_CHANGED):      patata_config_t *patata_config = patata_get_config();
                                            patata_config->autoconnect = lv_switch_get_state( obj );
                                            break;
    } */
}

static void patata_widget_onoff_event_handler(lv_obj_t *obj, lv_event_t event)
{
   /*  switch ( event ) {
        case ( LV_EVENT_VALUE_CHANGED ):    patata_config_t *patata_config = patata_get_config();
                                            patata_config->widget = lv_switch_get_state( obj );
                                            if ( patata_config->widget ) {
                                                patata_add_widget();
                                            }
                                            else {
                                                patata_remove_widget();
                                            }
                                            break;
    } */
}
