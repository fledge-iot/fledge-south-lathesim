/*
 * Fledge south plugin.
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */
#include <plugin_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <logger.h>
#include <plugin_exception.h>
#include <config_category.h>
#include <version.h>
#include <lathe.h>

using namespace std;

#define PLUGIN_NAME "lathesim"

static const char *default_config = QUOTE({
	"plugin" : { 
		"description" : "A simulation of a lathe",
		"type" : "string",
		"default" : PLUGIN_NAME,
		"readonly" : "true"
		},
	"name" : { 
		"description" : "Lathe name, used as the stem for the asset names",
		"type" : "string",
		"default" : "lathe",
		"displayName": "Lathe Name",
		"mandatory": "true",
		"order" : "1"
		},
	"spinup" : {
		"description" : "Time in seconds for lathe to come up to speed",
		"type" : "integer",
		"default" : "5",
		"displayName": "Spin up time",
		"mandatory": "true",
		"order" : "2"
		},
	"runtime" : {
		"description" : "Cutting of job after spinup",
		"type" : "integer",
		"default" : "45",
		"displayName": "Cutting time",
		"mandatory": "true",
		"order" :"3" 
		},
	"spindown" : {
		"description" : "Time in seconds for lathe to come to rest",
		"type" : "integer",
		"default" : "6",
		"displayName": "Spin down time",
		"mandatory": "true",
		"order" : "4"
		},
	"idletime" : {
		"description" : "Idle time between cutting jobs",
		"type" : "integer",
		"default" : "15",
		"displayName": "Idle time",
		"mandatory": "true",
		"order" :"5" 
		},
	"rpm" : {
		"description" : "Nomimal RPM for cutting",
		"type" : "integer",
		"default" : "500",
		"displayName": "RPM",
		"mandatory": "true",
		"order" : "6"
		},
	"current" : {
		"description" : "Nomimal current for cutting",
		"type" : "integer",
		"default" : "750",
		"displayName": "Current",
		"mandatory": "true",
		"order" : "7"
		}
	});
		  
/**
 * The Lathe simualtor  plugin interface
 */
extern "C" {

/**
 * The plugin information structure
 */
static PLUGIN_INFORMATION info = {
	PLUGIN_NAME,              // Name
	VERSION,                  // Version
	0,    			  // Flags
	PLUGIN_TYPE_SOUTH,        // Type
	"2.0.0",                  // Interface version
	default_config            // Default configuration
};

/**
 * Return the information about this plugin
 */
PLUGIN_INFORMATION *plugin_info()
{
	return &info;
}

/**
 * Initialise the plugin, called to get the plugin handle
 */
PLUGIN_HANDLE plugin_init(ConfigCategory *config)
{
Lathe *lathe = new Lathe(config);

	lathe->start();
	return (PLUGIN_HANDLE)lathe;
}

/**
 * Start the Async handling for the plugin
 */
void plugin_start(PLUGIN_HANDLE *handle)
{
}

/**
 * Poll for a plugin reading
 */
std::vector<Reading *> *plugin_poll(PLUGIN_HANDLE *handle)
{
Lathe *lathe = (Lathe *)handle;

	return lathe->takeReading();
}

/**
 * Reconfigure the plugin
 */
void plugin_reconfigure(PLUGIN_HANDLE *handle, string& newConfig)
{
ConfigCategory	config("lathe", newConfig);
Lathe		*lathe = (Lathe *)*handle;

	lathe->reconfigure(config);
}

/**
 * Shutdown the plugin
 */
void plugin_shutdown(PLUGIN_HANDLE *handle)
{
Lathe *lathe = (Lathe *)handle;

	delete lathe;
}
};
