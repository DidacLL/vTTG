/****************************************************************************
 *   Sep 3 23:05:42 2020
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
#include "patata_setup.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"

#include "utils/json_psram_allocator.h"
/*
 * patata app config
 */
//patata_config_t patata_config;
/*
 * app tiles
 */
uint32_t patata_app_main_tile_num;
uint32_t patata_app_setup_tile_num;
/*
 * app and widget icon
 */
icon_t *patata_app = NULL;
icon_t *patata_widget = NULL;
/*
 * declare callback functions for the app and widget icon to enter the app
 */
LV_IMG_DECLARE(patata_64px);
/*
 * automatic register the app setup function with explicit call in main.cpp
 */
static void enter_patata_app_event_cb( lv_obj_t * obj, lv_event_t event );
/*
 * automatic register the app setup function with explicit call in main.cpp
 */
static int registed = app_autocall_function( &patata_app_setup, 12 );           /** @brief app autocall function */
/**
 * @brief setup routine for patata app
 */
void patata_app_setup( void ) {
    /*
     * check if app already registered for autocall
     */
    if( !registed ) {
        return;
    }
    #if defined( ONLY_ESSENTIAL )
        return;
    #endif
    /*
     * load config
     */
    //patata_config.load();
    /*
     * register 2 tiles and get tile numbers and save it for later use
     */
    patata_app_main_tile_num = mainbar_add_app_tile( 1, 1, "Patata App" );
    patata_app_setup_tile_num = mainbar_add_setup_tile( 1, 2, "Patata App" );
    /*
     * register app icon on the app tile
     * set your own icon and register her callback to activate by an click
     * remember, an app icon must have an size of 64x64 pixel with an alpha channel
     * use https://lvgl.io/tools/imageconverter to convert your images and set "true color with alpha" to get fancy images
     * the resulting c-file can put in /app/examples/images/ and declare it like LV_IMG_DECLARE( your_icon );
     */
    patata_app = app_register( "patata-\napp", &patata_64px, enter_patata_app_event_cb );

   /*  if ( patata_config.widget ) {
        patata_add_widget();
    } */
    /*
     * init main and setup tile, see sailing_main.cpp and sailing_setup.cpp
     */
    patata_main_tile_setup( patata_app_main_tile_num );
    patata_setup_tile_setup( patata_app_setup_tile_num );
}
/**
 * @brief get the app main tile number
 * 
 * @return uint32_t 
 */
uint32_t patata_get_app_main_tile_num( void ) {
    return( patata_app_main_tile_num );
}
/**
 * @brief get the app setup tile number
 * 
 * @return uint32_t 
 */
uint32_t patata_get_app_setup_tile_num( void ) {
    return( patata_app_setup_tile_num );
}
/**
 * @brief get the app icon structure
 * 
 * @return icon_t* 
 */
icon_t *patata_get_app_icon( void ) {
    return( patata_app );
}
/**
 * @brief get the widget icon structure
 * 
 * @return icon_t* 
 */
icon_t *patata_get_widget_icon( void ) {
    return( patata_widget );
}
/**
 * @brief call back function when enter the app
 * 
 * @return icon_t* 
 */
static void enter_patata_app_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( patata_app_main_tile_num, LV_ANIM_OFF, true );
                                        break;
    }    
}
/**
 * @brief get the patata config structure
 * 
 * @return patata_config_t* 

patata_config_t *patata_get_config( void ) {
    return( &patata_config );
} */
/**
 * @brief add the widget to the mainbar
 * 
 * @return true 
 * @return false 
 */
bool patata_add_widget( void ) {
    if ( patata_widget == NULL ) {
        patata_widget = widget_register( "n/a", &patata_64px, enter_patata_app_event_cb );
        widget_hide_indicator( patata_widget );
        if ( patata_widget != NULL ) {
            return( true );
        }
        else {
            return( false );
        }
    }
    return( true );
}
/**
 * @brief remove the widget from the mainbar
 * 
 * @return true 
 * @return false 
 */
bool patata_remove_widget( void ) {
    patata_widget = widget_remove( patata_widget );
    return( true );
}
