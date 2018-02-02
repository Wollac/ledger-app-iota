#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "hash_file.h"
#include "test_constants.h"
#include "vendor/iota/addresses.h"
#include "vendor/iota/conversion.h"

#define MAX_SECURITY 3

static void seed_address(const char *seed_chars, uint32_t idx, uint8_t security,
                         char *address_chars)
{
    if (security > MAX_SECURITY || strlen(seed_chars) != NUM_HASH_TRYTES) {
        address_chars = "";
        return;
    }

    uint32_t seed_bigint[NUM_HASH_BIGINTS];
    chars_to_bigints(seed_chars, seed_bigint, NUM_HASH_TRYTES);

    uint32_t address_bigint[NUM_HASH_BIGINTS];
    get_public_addr(seed_bigint, idx, security, address_bigint);

    bigints_to_chars(address_bigint, address_chars, NUM_HASH_BIGINTS);
}

static void test_address(const char *seed, uint32_t idx, const char *expected)
{
    char output[MAX_NUM_TRYTES + 1];
    seed_address(seed, idx, 2, output);

    assert_string_equal(output, expected);
}

static void test_peter_seed(void **state)
{
    const char *seed =
        "PETERPETERPETERPETERPETERPETERPETERPETERPETERPETERPETERPETERPETERPETER"
        "PETERPETERR";
    const char *address[] = {
        "GUIOZDLUNXIGC9DCV9ZIEDBWRHHPILAYOYRVPTFPRAUZWLWDIXBSPCZGENHWDFHMQGCTOK"
        "MXITVVDMEFB",
        "MTPYSBLSL9HENRQKP9IPYYZTHEOECLXGYMZIYYUCYAPZYFAECX9ZSFOSFMDNYQAPYHVMTV"
        "UX9HNNUKOB9",
        "RKPTFXPROTSKXBKXLNSLOPOQGWASCLAECQQRWOKCJPNYHIFBUJXE9GHQJPIZHKYXXHC9BZ"
        "JPHAROKBGSD",
        "JYJFIYFNTDPTPGSJWAKUFK9OLTISGIKSQPTLIVRVHLHRRCSJCEFQRTGWVTBUQFXHFRZICM"
        "FDTPDKNKDFW",
        "WPF9CTKYVMEWXHXL9NKR9XON9TPBP9UNM9FPWBUISVSHNULLVHSU9PMBNNR9FSZUPCNBXG"
        "JWLGRKKSLHW"};

    uint32_t idx = (uintptr_t)*state;
    assert_in_range(idx, 0, sizeof(address) / sizeof(address[0]) - 1);

    test_address(seed, idx, address[idx]);
}

static void test_n_addresses_for_seed(void **state)
{
    (void)state;  // unused

    void test(char *hashes[])
    {
        for (uint32_t idx = 0; idx < 4; idx++) {
            test_address(hashes[0], idx, hashes[idx + 1]);
        }
    }

    test_for_each_line("generateNAddressesForSeed", test);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_prestate(test_peter_seed, (uint32_t *)0),
        cmocka_unit_test_prestate(test_peter_seed, (uint32_t *)1),
        cmocka_unit_test_prestate(test_peter_seed, (uint32_t *)2),
        cmocka_unit_test_prestate(test_peter_seed, (uint32_t *)3),
        cmocka_unit_test_prestate(test_peter_seed, (uint32_t *)4),
        cmocka_unit_test(test_n_addresses_for_seed)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
