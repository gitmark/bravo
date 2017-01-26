#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <bravo/string_utils.h>
#include <check.h>

using namespace std;
using namespace bravo;

void setup(void)
{
}

void teardown(void)
{
}

START_TEST(test_trim1)
{
    string s1(" hello  ");
    const char *s2 = "hello";
    string s3 = trim(s1);
    
    ck_assert_int_eq((int)s3.size(), (int)strlen(s2));
    ck_assert_str_eq(s3.c_str(), s2);
}
END_TEST

START_TEST(test_trim2)
{
    string s1("hello  \t");
    const char *s2 = "hello";
    string s3 = trim(s1);
    
    ck_assert_int_eq((int)s3.size(), (int)strlen(s2));
    ck_assert_str_eq(s3.c_str(), s2);
}
END_TEST

START_TEST(test_trim3)
{
    string s1(" \r\n hello");
    const char *s2 = "hello";
    string s3 = trim(s1);
    
    ck_assert_int_eq((int)s3.size(), (int)strlen(s2));
    ck_assert_str_eq(s3.c_str(), s2);
}
END_TEST

START_TEST(test_trim4)
{
    string s1(" 22\t hello");
    const char *s2 = "22\t hello";
    string s3 = trim(s1);
    
    ck_assert_int_eq((int)s3.size(), (int)strlen(s2));
    ck_assert_str_eq(s3.c_str(), s2);
}
END_TEST

START_TEST(test_trim5)
{
    string s1(" 22\t hello  \r ");
    string s4 = s1;
    const char *s2 = "22\t hello";
    string s3 = trim(s1);
    
    ck_assert_int_eq((int)s3.size(), (int)strlen(s2));
    ck_assert_str_eq(s3.c_str(), s2);
    ck_assert_str_eq(s1.c_str(), s4.c_str());
}
END_TEST


Suite * bravo_suite(void)
{
    Suite *s;
    TCase *tc_trim;

    s = suite_create("Bravo String Utils");

    tc_trim = tcase_create("String Trim");

    tcase_add_checked_fixture(tc_trim, setup, teardown);
    tcase_add_test(tc_trim, test_trim1);    
    tcase_add_test(tc_trim, test_trim2);
    tcase_add_test(tc_trim, test_trim3);
    tcase_add_test(tc_trim, test_trim4);
    tcase_add_test(tc_trim, test_trim5);
    suite_add_tcase(s, tc_trim);

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
     
     
     
     
     
     
     
