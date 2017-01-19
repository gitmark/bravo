// https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Autotest-Logs.html#Autotest-Logs
/*
mae notes
testsuite creates testsuite.log, log file is named after the script
usually more info than you need.
don't set env vars before calling a test command, instead pass info as command line args, this way the logs will show you what vars were changed
cross compiling tests to run on another machine is not easy
host = build

/////////////////
Previous: testsuite Scripts, Up: Using an Autotest Test Suite

19.1.2 Autotest Logs

When run, the test suite creates a log file named after itself, e.g., a test suite named testsuite creates testsuite.log. It contains a lot of information, usually more than maintainers actually need, but therefore most of the time it contains all that is needed:

command line arguments
A bad but unfortunately widespread habit consists of setting environment variables before the command, such as in ‘CC=my-home-grown-cc ./testsuite’. The test suite does not know this change, hence (i) it cannot report it to you, and (ii) it cannot preserve the value of CC for subsequent runs. Autoconf faced exactly the same problem, and solved it by asking users to pass the variable definitions as command line arguments. Autotest requires this rule, too, but has no means to enforce it; the log then contains a trace of the variables that were changed by the user. 

ChangeLog excerpts the topmost lines of all the ChangeLog files found in the source hierarchy. This is especially useful when bugs are reported against development versions of the package, since the version string does not provide sufficient information to know the exact state of the sources the user compiled. Of course, this relies on the use of a ChangeLog. 
build machine
Running a test suite in a cross-compile environment is not an easy task, since it would mean having the test suite run on a machine build, while running programs on a machine host. It is much simpler to run both the test suite and the programs on host, but then, from the point of view of the test suite, there remains a single environment, host = build. The log contains relevant information on the state of the build machine, including some important environment variables. 
tested programs
The absolute file name and answers to --version of the tested programs (see Writing Testsuites, AT_TESTED). 
configuration log
The contents of config.log, as created by configure, are appended. It contains the configuration flags and a detailed report on the configuration itself.
 */
