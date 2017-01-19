// https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/testsuite-Scripts.html#testsuite-Scripts
/*
mae notes
autom4te creates test scripts. the test scripts created do not require autm4te or GNU M4.

May need two make setups. One for tests and one for the app. ie perhaps Makefile.am in the top_srcdir should not reference Makefile.am in the tests dir.

Tests should be executed in groups of similar tests for the same feature.
package.m4 is automatically included if found.

One approach is to include AC_INIT and other local tests in atlocal.in and
only put m4_include() statements in testsuite.at

a debugging dir is left behind for each test that fails.

AC_CONFIG_TESTDIR creates atconfig which contains config info for the tests

AC_CONFIG_FILES reads atconfig.in and atlocal.in and creates atconfig and atlocal

								atconfig
[atlocal.in] -> config.status <
								[atlocal]
/////////////////////////////////
Next: Autotest Logs, Up: Using an Autotest Test Suite

19.1.1 testsuite Scripts

Generating testing or validation suites using Autotest is rather easy. The whole validation suite is held in a file to be processed through autom4te, itself using GNU M4 under the hood, to produce a stand-alone Bourne shell script which then gets distributed. Neither autom4te nor GNU M4 are needed at the installer's end.

Each test of the validation suite should be part of some test group. A test group is a sequence of interwoven tests that ought to be executed together, usually because one test in the group creates data files that a later test in the same group needs to read. Complex test groups make later debugging more tedious. It is much better to keep only a few tests per test group. Ideally there is only one test per test group.

For all but the simplest packages, some file such as testsuite.at does not fully hold all test sources, as these are often easier to maintain in separate files. Each of these separate files holds a single test group, or a sequence of test groups all addressing some common functionality in the package. In such cases, testsuite.at merely initializes the validation suite, and sometimes does elementary health checking, before listing include statements for all other test files. The special file package.m4, containing the identification of the package, is automatically included if found.

A convenient alternative consists in moving all the global issues (local Autotest macros, elementary health checking, and AT_INIT invocation) into the file local.at, and making testsuite.at be a simple list of m4_includes of sub test suites. In such case, generating the whole test suite or pieces of it is only a matter of choosing the autom4te command line arguments.

The validation scripts that Autotest produces are by convention called testsuite. When run, testsuite executes each test group in turn, producing only one summary line per test to say if that particular test succeeded or failed. At end of all tests, summarizing counters get printed. One debugging directory is left for each test group which failed, if any: such directories are named testsuite.dir/nn, where nn is the sequence number of the test group, and they include:

a debugging script named run which reruns the test in debug mode (see testsuite Invocation). The automatic generation of debugging scripts has the purpose of easing the chase for bugs.
all the files created with AT_DATA
all the Erlang source code files created with AT_CHECK_EUNIT
a log of the run, named testsuite.log
In the ideal situation, none of the tests fail, and consequently no debugging directory is left behind for validation.

It often happens in practice that individual tests in the validation suite need to get information coming out of the configuration process. Some of this information, common for all validation suites, is provided through the file atconfig, automatically created by AC_CONFIG_TESTDIR. For configuration information which your testing environment specifically needs, you might prepare an optional file named atlocal.in, instantiated by AC_CONFIG_FILES. The configuration process produces atconfig and atlocal out of these two input files, and these two produced files are automatically read by the testsuite script.

Here is a diagram showing the relationship between files.

Files used in preparing a software package for distribution:

                     [package.m4] -->.
                                      \
     subfile-1.at ->.  [local.at] ---->+
         ...         \                  \
     subfile-i.at ---->-- testsuite.at -->-- autom4te* -->testsuite
         ...         /
     subfile-n.at ->'
Files used in configuring a software package:

                                          .--> atconfig
                                         /
     [atlocal.in] -->  config.status* --<
                                         \
                                          `--> [atlocal]
Files created during test suite execution:

     atconfig -->.                    .--> testsuite.log
                  \                  /
                   >-- testsuite* --<
                  /                  \
     [atlocal] ->'                    `--> [testsuite.dir]
*/
