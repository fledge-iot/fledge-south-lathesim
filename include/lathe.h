#ifndef _LATHE_H
#define _LATHE_H
/*
 * Fledge south service plugin
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */
#include <reading.h>
#include <string>
#include <config_category.h>
#include <mutex>
#include <thread>
#include <sys/time.h>

class Lathe {
	public:
		Lathe(ConfigCategory *);
		~Lathe();
		void			start();
		void			stop();
		std::vector<Reading *> *takeReading();
		void			reconfigure(ConfigCategory& config);
	private:
		void		configure(ConfigCategory *config);
		long		getNumericConfig(ConfigCategory *config, const std::string& name, long defaultValue);
		void		newState();
		long		cycleOffset();
		void		getState(std::string& str);
		/**
		 * Following block are the configuration variables for
		 * the simulation.
		 */
		std::string	m_name;
		long		m_spinup;
		long		m_runtime;
		long		m_idletime;
		long		m_spindown;
		long		m_rpm;
		long		m_current;
		/**
		 * Following are the current state of the simulation
		 */
		std::mutex	m_configMutex;
		struct timeval	m_simStart;	// Time at start of current simulation cycle
		long		m_speed;	// Current speed of lathe
		long		m_x;		// Current X position of cutting tool
		long		m_depth;	// Current depth of cutting tool
		enum		{ SpiningUp, Cutting, SpiningDown, Idle }
				m_state;	// Current state of lathe
		long		m_milliamps;	// Current dtae of the lathe
		long		m_rms;		// RMS of the vibration
		double		m_freq;		// Peak vibration frequency
};
#endif
