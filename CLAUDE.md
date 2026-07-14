# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LinKNX is a KNX home automation platform written in C++ that acts as a logic engine and automation server for KNX building control systems. It connects to KNX networks via EIB/KNX interfaces and provides rule-based automation, timers, persistent storage, and remote control via XML-based protocols.

**Key capabilities:**
- KNX object management and monitoring
- Rule-based automation with conditions and actions
- Time-based scheduling with timer support
- XML-based remote control interface (TCP/IP and Unix sockets)
- Persistent storage backends (MySQL, InfluxDB)
- SMS and email gateway integration
- Lua scripting support for custom logic
- MQTT support for IoT integration

## Build System

LinKNX uses GNU Autotools. The project requires several dependencies that are detected and conditionally compiled at configure time.

### Quick start - Building from git
```bash
# 1. Install dependencies (Ubuntu/Debian example)
sudo apt install build-essential libcppunit-dev gettext \
  libcurl4-openssl-dev libesmtp-dev liblog4cpp5-dev \
  liblua5.1-0-dev libmariadb-dev-compat libjsoncpp-dev

# 2. Build and install pthsem (required dependency)
# Get it from: https://github.com/linknx/pthsem
cd pthsem
./configure --disable-shared --prefix=/usr/local
make install

# 3. Build LinKNX
cd linknx
autoreconf --install
./configure --with-pth=/usr/local
make
sudo make install
```

### Configure options
The build system auto-detects optional features. Override with:
- `--with-pth=DIR` - Path to pthsem installation (required)
- `--with-mysql[=DIR]` - MySQL persistent storage support
- `--enable-smtp` - libesmtp email support (or `--enable-smtp=static` for static linking)
- `--with-cppunit` - Build unit tests (requires CppUnit >= 1.9.6)
- `--with-log4cpp` - Advanced logging with Log4cpp >= 1.0
- `--with-lua` - Lua scripting engine >= 5.1
- `--with-mqtt` - MQTT support via libmosquitto >= 2.1
- `--with-jsoncpp` - InfluxDB persistence support (requires jsoncpp >= 1.7.2 + libcurl)

### Running tests
```bash
make check
```

This builds and runs the CppUnit test suite if `--with-cppunit` was configured. Tests require `TZ=CET` environment variable (tests check DST transitions specific to Central European Time).

## Architecture

### Core Components

**Services (src/services.h/cpp)**
Singleton managing all subsystems. Entry point for accessing:
- `KnxConnection` - KNX network interface
- `TimerManager` - Scheduling and periodic tasks
- `XmlServer` - Remote control interface
- `PersistentStorage` - Data persistence
- `SmsGateway` / `EmailGateway` - Notification services
- `ExceptionDays` - Calendar exception handling
- `LocationInfo` - Sunrise/sunset calculations

**ObjectController (src/objectcontroller.h/cpp)**
Central registry for all KNX objects. Manages:
- Object lifecycle (creation, deletion, lookup)
- Object types (switching, dimming, temperature, time, etc.)
- Value conversion to/from KNX data point types (DPT)
- Change notification to listeners
- Import/export to XML configuration

Objects derive from `Object` base class and implement `ObjectValue` for type-specific value handling.

**RuleServer (src/ruleserver.h/cpp)**
Automation rule engine. Key abstractions:
- `Rule` - Associates conditions with actions
- `Condition` - Boolean expressions (and/or/not/object/timer/script/etc.)
- `Action` - Operations to execute (set-value/send-sms/send-email/script/etc.)
- `ActionList` - Sequential action execution

Rules can be stateful (active/inactive) and support delayed/repeated actions.

**KnxConnection (src/knxconnection.h/cpp)**
Manages connection to EIB/KNX daemon. Handles:
- Group address read/write/response
- Connection state monitoring
- Read/write request queuing
- Callback dispatch to Object instances

Uses EIBD client library (eibclient.c/h in src/ and include/).

**XmlServer (src/xmlserver.h/cpp)**
TCP (XmlInetServer) or Unix socket (XmlUnixServer) server for XML-based control protocol.
- `ClientConnection` threads handle individual clients
- Implements ChangeListener to push object updates to clients
- Protocol allows read/write objects, query status, manage rules/timers

**TimerManager (src/timermanager.h/cpp)**
Schedules one-shot and periodic tasks. Supports:
- Fixed and variable time specifications
- Sunrise/sunset-relative scheduling
- Day/week/month patterns with exception days
- Cron-like periodicity

**PersistentStorage (src/persistentstorage.h/cpp)**
Abstract interface with MySQL and InfluxDB backends:
- `write(id, value)` / `read(id, defval)` - Key-value storage for object state
- `writelog(id, value)` - Time-series logging

**IOPortManager (src/ioport.h/cpp)**
Manages external I/O connections (serial, TCP, UDP). Features:
- Bidirectional communication with external devices
- Regex-based message parsing
- TxAction for sending data, RxCondition for receiving
- Used for integration with non-KNX devices (RS232, network protocols)

### Thread Model

Uses GNU Pth (portable threading) via pthsem library - a cooperative user-space threading library. Key characteristics:
- All threads run in a single OS thread (cooperative scheduling)
- Requires pthsem-aware I/O functions for proper thread switching
- Thread primitives in src/threads.h/cpp wrap pthsem

Main threads:
- Main thread runs Services initialization and signal handling
- XmlServer spawns ClientConnection threads for each client
- TimerManager runs timer dispatch loop
- KnxConnection runs receive loop for KNX bus events
- IOPort threads for serial/network I/O (ioport.h/cpp)

### Configuration

Primary config is XML (conf/linknx.xml). Schema defined in conf/linknx.xsd. See conf/sample.xml for a complete example.

Structure:
```xml
<config>
  <services>
    <knxconnection url="ip:127.0.0.1:6720"/>  <!-- or local:/tmp/eib -->
    <xmlserver type="inet" port="1028"/>
    <persistence type="mysql" host="..." user="..." pass="..." db="..."/>
  </services>
  <objects>
    <!-- KNX objects with group addresses -->
    <object id="light_kitchen" gad="1/1/11" type="1.001">Kitchen Light</object>
    <object id="temp_living" gad="1/2/10" type="EIS5" init="request"/>
  </objects>
  <rules>
    <rule id="motion_light" init="true">
      <condition type="object" id="motion_sensor" value="on"/>
      <actionlist>
        <action type="set-value" id="light" value="on"/>
      </actionlist>
    </rule>
  </rules>
  <ioports>
    <ioport id="serial1" type="tcp" host="..." port="..."/>
  </ioports>
</config>
```

LinKNX can read/write its configuration via XML protocol at runtime. Objects support:
- `init="request"` - read initial value from KNX bus on startup
- `init="persist"` - restore value from persistent storage
- `gad` - KNX group address (main address for read/write)
- `listener` child elements - additional group addresses to monitor

### Logging

Uses log4cpp when available (HAVE_LOG4CPP), otherwise falls back to basic Logger class (src/logger.h/cpp). Log categories mirror component hierarchy.

## Development Notes

### Adding a new Object type
1. Define ObjectValue subclass in objectcontroller.h
2. Implement value conversion (toString, set, compare, equals)
3. Add Object subclass with KNX DPT encoding/decoding
4. Register factory in ObjectController::create()
5. Update linknx.xsd schema for XML config support
6. Add unit tests in test/ObjectTest.cpp

Object types map to KNX Data Point Types (DPT). Legacy type names like "EIS5" (temperature) are aliases for DPT numbers.

### Adding a new Condition type
1. Define Condition subclass in ruleserver.h
2. Implement evaluate(), importXml(), exportXml(), statusXml()
3. Register factory in Condition::create()
4. Update linknx.xsd for XML config
5. Add unit tests in test/RuleTest.cpp

Conditions can register as ChangeListeners on Objects to get notified on value changes.

### Adding a new Action type
1. Define Action subclass in ruleserver.h (or ioport.h for I/O-related actions)
2. Implement execute() - runs in Action's own thread
3. Register factory in Action::create()
4. Update linknx.xsd
5. Add unit tests

Common action types: SetValueAction, SendReadRequestAction, SendSmsAction, SendEmailAction, TxAction (I/O port transmission).

### XML Protocol Extension
ClientConnection::processMessage() in xmlserver.cpp dispatches XML commands. Add new command handling there. The protocol is text-based XML over TCP/Unix sockets.

### Conditional Compilation Guards
Feature support is guarded by autoconf defines:
- `HAVE_MYSQL` - MySQL support
- `HAVE_LIBESMTP` - SMTP/email support
- `HAVE_LOG4CPP` - Log4cpp logging
- `HAVE_LUA` - Lua scripting
- `HAVE_MQTT` - MQTT support
- `SUPPORT_INFLUXDB` - InfluxDB persistence (requires libcurl + jsoncpp)

Check these before referencing optional features.

## Testing

CppUnit-based unit tests in test/:
- ObjectTest.cpp - Object value conversion and DPT encoding
- RuleTest.cpp - Rule condition evaluation
- TimeSpecTest.cpp - Time specification parsing
- PeriodicTaskTest.cpp - Timer scheduling logic
- XmlServerTest.cpp - XML protocol handling

Run with `make check` after configuring with `--with-cppunit`.

## Common Patterns

**Singleton Services**
Most subsystems accessed via Services::instance()->getXxx(). Reset with Services::reset() in tests.

**Factory Pattern**
All extensible types use static create() factories:
```cpp
Object* obj = Object::create("1.001");  // by DPT type
Condition* cond = Condition::create("and", changeListener);
Action* action = Action::create("set-value");
```

**XML Import/Export (TiCpp)**
All persistent entities implement importXml(ticpp::Element*) and exportXml(ticpp::Element*) for configuration serialization. Uses TiCpp (TinyXML++) library (ticpp/ directory) for XML parsing. TiCpp uses exceptions for error handling - wrap XML operations in try/catch.

**Change Notification**
Objects implement ChangeListener callbacks. Register via Object::addChangeListener() to receive onChange() on value updates. Used by rules to trigger on object state changes.

**Logger Usage**
Static logger members: `Logger& logger_m = Logger::getInstance("ComponentName");`
Then: `logger_m.infoStream() << "message";`

**Thread Class**
Subclass Thread (src/threads.h) and implement Run(pth_sem_t* stop). The stop semaphore signals when thread should terminate.

## Key Dependencies

- **pthsem** - GNU Portable Threads with semaphores (cooperative threading, REQUIRED)
- **TiCpp** - TinyXML++ for XML parsing (bundled in ticpp/)
- **EIBD client library** - EIB/KNX bus communication (bundled in include/, src/eibclient.c)
- **b64** - Base64 encoding for email attachments (bundled in b64/)
- **libcurl** - HTTP client for InfluxDB (optional)
- **libesmtp** - SMTP email support (optional)
- **log4cpp** - Advanced logging (optional, fallback to basic logger)
- **lua** - Scripting engine for custom conditions/actions (optional)
- **libmosquitto** - MQTT protocol support (optional)
- **jsoncpp** - JSON parsing for InfluxDB (optional)
- **mysql client** - MySQL persistence backend (optional)

## Project History

Originally hosted on SourceForge, migrated to GitHub. Version numbering is 0.0.1.x (currently 0.0.1.39). Active CI tests against gcc 10, 11, 12 on Ubuntu.