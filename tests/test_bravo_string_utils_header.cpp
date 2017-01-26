#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <string>
#include <bravo/string_utils.h>
#include <bravo/hex.h>
extern "C" {
#ifdef _WIN32
#define pid_t int
#endif
#include <check.h>
}

//#include <config.h>

using namespace std;
using namespace bravo;

void setup(void)
{
}

void teardown(void)
{
}

START_TEST(test_trim)
{
    string s1(" hello  ");
    const char *s2 = "hello";
    string s3 = trim(s1);
    
    ck_assert_int_eq((int)s3.size(), (int)strlen(s2));
    ck_assert_str_eq(s3.c_str(), s2);
}
END_TEST

START_TEST(test_hex_to_char)
{
    ck_assert_msg(0x0A == hex_to_char("0A"), "Hex error, 0A != 0A");
}
END_TEST

START_TEST(test_char_to_hex)
{
    if (char_to_hex(0x0D) != "0D")
    {
        ck_abort_msg("Hex error, 0D != 0D");
    }
}
END_TEST

Suite * bravo_suite(void)
{
    Suite *s;
    TCase *tc_string_utils;
    TCase *tc_hex;

    s = suite_create("Bravo");

    tc_string_utils = tcase_create("String Utils");

    tcase_add_checked_fixture(tc_string_utils, setup, teardown);
    tcase_add_test(tc_string_utils, test_trim);
    suite_add_tcase(s, tc_string_utils);

    tc_hex = tcase_create("Hex Conversions");

    tcase_add_test(tc_hex, test_hex_to_char);
    tcase_add_test(tc_hex, test_char_to_hex);
    suite_add_tcase(s, tc_hex);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = bravo_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
     
     
     
     
     
     
     
