/*
 * Fledge south service plugin
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */
#include <lathe.h>
#include <reading.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

/**
 * Constructor for the Lathe "sensor"
 */
Lathe::Lathe(ConfigCategory *config)
{
	this->configure(config);
}

/**
 * Destructor for the lathe "sensor"
 */
Lathe::~Lathe()
{
	lock_guard<mutex> guard(m_configMutex);
}

/**
 * Take a reading from the lathe "sensor"
 */
vector<Reading *> *Lathe::takeReading()
{
vector<Reading *> *readings = new vector<Reading *>();
static bool	sendVibration = false;
static time_t	lastTemp = 0;

	lock_guard<mutex> guard(m_configMutex);
	newState();	// Move on the state of the simulation

	DatapointValue dpv_speed(m_speed);
	Datapoint *dp = new Datapoint("rpm", dpv_speed);
	Reading *lathe = new Reading(m_name, dp);
	DatapointValue dpv_x(m_x);
	lathe->addDatapoint(new Datapoint("x", dpv_x));
	DatapointValue dpv_depth(m_depth);
	lathe->addDatapoint(new Datapoint("depth", dpv_depth));
	string state;
	getState(state);
	DatapointValue dpv_state(state);
	lathe->addDatapoint(new Datapoint("state", dpv_state));
	readings->push_back(lathe);

	DatapointValue dpv_ma(m_milliamps);
	Reading *current = new Reading(m_name + "Current", new Datapoint("current", dpv_ma));
	readings->push_back(current);

	/*
	 * Vibration is only send ever other time a reading is taken. This is
	 * to simulate a sensor that reports at a different rate to the other
	 * sensors attached to the lathe.
	 */
	if (sendVibration)
	{
		DatapointValue dpv_rms(m_rms);
		Reading *vibration = new Reading(m_name + "Vibration", new Datapoint("rms", dpv_rms));
		DatapointValue dpv_freq(m_freq);
		vibration->addDatapoint(new Datapoint("frequency", dpv_freq));
		readings->push_back(vibration);
		sendVibration = false;
	}
	else
	{
		sendVibration = true;
	}

	/*
	 * Once a second we will send temperature data to simulate a
	 * Flir camera point at the lathe.
	 */
	time_t t = time(0);
	if (lastTemp != t)
	{
		lastTemp = t;
		double headstock = 24.0, tailstock = 20.5, gearbox = 25.4;
	        double motor = 27.6, tool = 18.0;
		long offset = cycleOffset();
		switch (m_state)
		{
			case Idle:
				gearbox += ((double)(rand() % 500) / 200.0) - 1.25;
				motor += ((double)(rand() % 500) / 200.0) - 1.25;
				headstock += ((double)(rand() % 200) / 100.0) - 1;
				tailstock += ((double)(rand() % 200) / 100.0) - 1;
				tool += ((double)(rand() % 200) / 100.0) - 1;
				break;
			case SpiningUp:
				gearbox += ((double)(rand() % 500) / 100.0) - 2.5;
				motor += ((double)(rand() % 500) / 100.0) - 2.5;
				headstock += ((double)(rand() % 200) / 100.0) - 1;
				tailstock += ((double)(rand() % 200) / 150.0) - 1;
				tool += ((double)(rand() % 200) / 100.0) - 1;
				gearbox += (double)offset / 2500.0;
				motor += (double)offset / 2000.0;
				break;
			case Cutting:
				gearbox += (double)(offset - (m_spinup * 1000)) / 8000.0;
				motor += (double)(offset - (m_spinup * 1000)) / 10000.0;
				tool += (double)(offset - (m_spinup * 1000)) / 4000.0;
				tool += ((double)(rand() % 600) / 100.0) - 3;
				headstock += ((double)(rand() % 200) / 100.0);
				tailstock += ((double)(rand() % 200) / 150.0);

				break;
			case SpiningDown:
				gearbox += ((double)(rand() % 500) / 400.0);
				motor += ((double)(rand() % 500) / 400.0);
				break;
		}
		DatapointValue dpv_gearbox(gearbox);
		Reading *temp = new Reading(m_name + "IR", new Datapoint("gearbox", dpv_gearbox));
		DatapointValue dpv_motor(motor);
		temp->addDatapoint(new Datapoint("motor", dpv_motor));
		DatapointValue dpv_headstock(headstock);
		temp->addDatapoint(new Datapoint("headstock", dpv_headstock));
		DatapointValue dpv_tailstock(tailstock);
		temp->addDatapoint(new Datapoint("tailstock", dpv_tailstock));
		DatapointValue dpv_tool(tool);
		temp->addDatapoint(new Datapoint("tool", dpv_tool));
		readings->push_back(temp);
	}
	
	return readings;
}

/**
 * Perform reconfiguration of the lathe plugin. This
 * merely takes out the mutex and calls the configure method.
 * The mutex is required as the reconfigure call can occur
 * on a thread that is seperate from the polling thread
 * and reconfiguring during a poll operation can cause
 * issues.
 *
 * @param config	The new configuration to apply
 */
void Lathe::reconfigure(ConfigCategory& config)
{
	lock_guard<mutex> guard(m_configMutex);
	configure(&config);
}

/**
 * Perform configuration of the lathe plugin
 *
 * @param config	The new configuration to apply
 */
void Lathe::configure(ConfigCategory *config)
{
	if (config->itemExists("name"))
		m_name = config->getValue("name");
	else
		m_name = "lathe";
	m_spinup = getNumericConfig(config, "spinup", 5);
	m_spindown = getNumericConfig(config, "spindown", 5);
	m_runtime = getNumericConfig(config, "runtime", 30);
	m_idletime = getNumericConfig(config, "idletime", 15);
	m_rpm = getNumericConfig(config, "rpm", 1000);
	m_current = getNumericConfig(config, "current", 1500);

	Logger::getLogger()->info("Lathe simulation cycle time %d",
			m_spinup + m_runtime + m_spindown + m_idletime);
}

/**
 * Start the sumulation run for the lathe
 */
void Lathe::start()
{
	Logger::getLogger()->debug("Lathe simulation starting");
	gettimeofday(&m_simStart, NULL);
	srand(time(0));
}

/**
 * Stop the sumulation run for the lathe
 */
void Lathe::stop()
{
}

/**
 * Move the lathe simulation on in real time. Simualtion states
 * are simply based on time and repeat continually.
 *
 * The values of the various data points are calculated based on
 * the current state of the lathe and time within that state. For
 * example the speed increases linearly during the spinup state 
 * from zero to the full RPM value defined. The oposite occurs
 * during spindown.
 *
 * The time, offset, is the number of milliseconds into the current
 * cycle of operation the simualtion is currently.
 */
void Lathe::newState()
{
	long offset = cycleOffset();
	if (offset < m_spinup * 1000)
	{
		m_state = SpiningUp;
		m_speed = (m_rpm * offset) / (m_spinup * 1000);
		m_milliamps = m_current + ((offset < 1500) ? (1500 - offset) / 10: 0);
		m_rms = offset / m_spinup;
		m_freq = (double)(m_rpm) / 60.0;
		m_depth = 40;
		m_x = 0;
	}
	else if (offset < (m_runtime + m_spinup) * 1000)
	{
		m_state = Cutting;
		m_speed = m_rpm + (m_rpm * (rand() % 10) - 5) / 100;
		m_milliamps = m_current + (rand() % 50);
		m_rms = 1000;
		m_rms += ((rand() % 20) - 10) * ((m_runtime - ((offset / 1000) + m_spinup)));
		m_freq = (double)(4 * m_rpm) / 60.0;
		double rtpercent = (offset - (m_spinup * 1000)) / (m_runtime * 10);
		m_x += (int)(rtpercent / 25) & 0x01 ? -1 : 1;
		m_depth = 40 - fabs(sin((double)m_x / 5.0) * 30.0);
	}
	else if (offset < (m_spindown + m_runtime + m_spinup) * 1000)
	{
		m_state = SpiningDown;
		long downoffset = offset - ((m_runtime + m_spinup) * 1000);
		m_speed = m_rpm - ((m_rpm * downoffset) / (m_spindown * 1000));
		m_milliamps = 150 + (m_current / (1 + downoffset));
		m_rms = 1000;
		m_freq = (double)(m_rpm) / 60.0;
		m_depth = 40;
		m_x = 0;
	}
	else
	{
		m_state = Idle;
		m_speed = 0;
		m_milliamps = 150;
		m_rms = 0;
		m_freq = 0;
		m_depth = 40;
		m_x = 0;
	}
	string state;
	getState(state);
	Logger::getLogger()->debug("Lathe simulation state %s", state.c_str());
}

/**
 * Extract a numeric value from the configuration
 *
 * @param config	The configuration category
 * @param name		The name of the item to extract
 * @param defaultValue	The default value to return if the item is missing from the category
 * @return	Numeric value of the configuration item
 */
long Lathe::getNumericConfig(ConfigCategory *config, const string& name, long defaultValue)
{
	if (!config->itemExists(name))
	{
		return defaultValue;
	}
	return strtol(config->getValue(name).c_str(), NULL, 10);
}

/**
 * Return the number of milliseconds into the simulation cycle we currently are
 *
 * @return cycle offset in milliseconds
 */
long Lathe::cycleOffset()
{
	struct timeval	tm1, tm2;

	gettimeofday(&tm1, NULL);

	timersub(&tm1, &m_simStart, &tm2);

	long seconds = tm2.tv_sec % (m_spinup + m_runtime + m_spindown + m_idletime);
	long offset = (1000 * seconds) + (tm2.tv_usec / 1000);

	return offset;
}

/**
 * Fetch a string representation of the state of the Lathe
 */
void Lathe::getState(string& str)
{
	switch (m_state)
	{
		case SpiningUp:
			str = "Spining Up";
			break;
		case SpiningDown:
			str = "Spining Down";
			break;
		case Cutting:
			str = "Cutting";
			break;
		case Idle:
			str = "Idle";
			break;
	}
}
