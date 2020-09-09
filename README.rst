Fledge Lathe Simulator South plugin

A plugin used to simulate the actions of a lathe

The simulator runs a pattern of operations that includes;

  - a spin-up period where the lathe spins up to speed from idle

  - a period where the lathe is doing some cutting of a work piece

  - a spin-down period where the lathe is slowing to a stop

  - an idle period where the work piece is removed and replace with a new billet


These actions then repeat themselves.

The plugin simulates three different south plugins connecting to three distinct sensors sources

  - the PLC controlling the lathe that gives details such as cutting depth, tool position, motor speed

  - a current sensor that measures the current draw from the lathe

  - a vibration sensor giving the RMS value of the vibration and the dominant vibration frequency

  - a thermal imaging device recordign temperature once a second from various parts of the lathe
