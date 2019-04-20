#include "x_core.h"
#include "nanos_types.h"
#include "x_screens.h"
#include "x_elements.h"
#include "nanos_misc.h"
#include "nanos_display.h"
#include "nanos_buttons.h"
#include "ui.h"
#include "iota/addresses.h"

#ifdef TARGET_NANOX

// TODO rename all functions

UI_SCREENS_NANOS current_screen;

UI_TEXT_CTX_NANOS ui_text;
UI_STATE_CTX_NANOS ui_state;

static void nanos_transition_state(unsigned int button_mask);
static void nanos_build_display();

// macros for button functions
BUTTON_FUNCTION(title)
BUTTON_FUNCTION(title_bold)
BUTTON_FUNCTION(menu)
BUTTON_FUNCTION(iota)
BUTTON_FUNCTION(back)
BUTTON_FUNCTION(dash)
BUTTON_FUNCTION(up)
BUTTON_FUNCTION(dn)
BUTTON_FUNCTION(updn)

void nanos_set_screen(UI_SCREENS_NANOS s)
{
    current_screen = s;
}

static void nanos_render()
{
    switch (current_screen) {
    case SCREEN_TITLE:
        UX_DISPLAY(bagl_ui_title_screen, NULL);
        break;
    case SCREEN_TITLE_BOLD:
        UX_DISPLAY(bagl_ui_title_bold_screen, NULL);
        break;
    case SCREEN_MENU:
        UX_DISPLAY(bagl_ui_menu_screen, NULL);
        break;
    case SCREEN_IOTA:
        UX_DISPLAY(bagl_ui_iota_screen, NULL);
        break;
    case SCREEN_BACK:
        UX_DISPLAY(bagl_ui_back_screen, NULL);
        break;
    case SCREEN_DASH:
        UX_DISPLAY(bagl_ui_dash_screen, NULL);
        break;
    default:
        THROW(INVALID_PARAMETER);
    }
}

static void x_get_screen()
{
}

static void nanos_ctx_initialize()
{
    MEMCLEAR(ui_text);
    MEMCLEAR(ui_glyphs);
    MEMCLEAR(ui_state);
}

void ui_init()
{
    nanos_ctx_initialize();
    ui_timeout_stop();

    ui_glyphs.glyph[TOTAL_GLYPHS] = '\0';

    ui_display_main_menu();
}

// Entry points for main to modify display
void ui_display_main_menu()
{
    state_go(STATE_MAIN_MENU, 0);
    backup_state();

    nanos_build_display();
    nanos_render();
    ui_force_draw();
}

void ui_display_getting_addr()
{
    nanos_set_screen(SCREEN_TITLE_BOLD);
    clear_display();

    write_display("    Generating", TOP);
    write_display("     Address...", BOT);

    display_glyphs(GLYPH_LOAD, GLYPH_NONE);

    backup_state();

    ui_state.state = STATE_IGNORE;

    nanos_render();
    ui_force_draw();
}

void ui_display_validating()
{
    nanos_set_screen(SCREEN_MENU);
    clear_display();

    write_display("    Validating...", MID);

    display_glyphs(GLYPH_LOAD, GLYPH_NONE);

    backup_state();

    ui_state.state = STATE_IGNORE;

    nanos_render();
    ui_force_draw();
}

void ui_display_recv()
{
    nanos_set_screen(SCREEN_TITLE_BOLD);
    clear_display();

    write_display("    Receiving", TOP);
    write_display("      Transaction...", BOT);

    display_glyphs(GLYPH_LOAD, GLYPH_NONE);

    backup_state();

    ui_state.state = STATE_IGNORE;

    nanos_render();
    ui_force_draw();
}

void ui_display_signing()
{
    nanos_set_screen(SCREEN_TITLE_BOLD);
    clear_display();

    write_display("    Signing", TOP);
    write_display("      Transaction...", BOT);

    display_glyphs(GLYPH_LOAD, GLYPH_NONE);

    backup_state();

    ui_state.state = STATE_IGNORE;

    nanos_render();
    ui_force_draw();
}

void ui_display_address(const unsigned char *addr_bytes)
{
    get_address_with_checksum(addr_bytes, ui_state.addr);
    state_go(STATE_DISP_ADDR_CHK, 0);

    nanos_build_display();
    nanos_render();
    ui_force_draw();
}

void ui_sign_tx()
{
    state_go(STATE_PROMPT_TX, 0);

    nanos_build_display();
    nanos_render();
}

void ui_reset()
{
    state_go(STATE_MAIN_MENU, 0);

    nanos_build_display();
    nanos_render();
    ui_force_draw();
}

void ui_restore()
{
    restore_state();

    nanos_build_display();
    nanos_render();
    ui_force_draw();
}

bool ui_lock_forbidden(void)
{
    // forbid app from locking during transaction (rely on tx timeout)
    switch (ui_state.state) {
        // BIP Path could be in tx or disp_addr
        // (backup state will tell us which)
    case STATE_BIP_PATH:
        if (ui_state.backup_state != STATE_PROMPT_TX)
            return false;
    case STATE_PROMPT_TX:
    case STATE_TX_ADDR:
        return true;
    default:
        return false;
    }
}

/* -------------------- SCREEN BUTTON FUNCTIONS ---------------
 ---------------------------------------------------------------
 --------------------------------------------------------------- */
static uint8_t nanos_translate_mask(unsigned int button_mask)
{
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        return BUTTON_L;
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        return BUTTON_R;
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT | BUTTON_LEFT:
        return BUTTON_B;
    default:
        return BUTTON_BAD;
    }
}

static void nanos_handle_button(uint8_t button_mask)
{
    int8_t array_sz;

    switch (ui_state.state) {
        /* ------------ STATE MAIN MENU -------------- */
    case STATE_MAIN_MENU:
        array_sz = button_main_menu(button_mask);
        break;
        /* ------------ STATE ABOUT -------------- */
    case STATE_ABOUT:
        array_sz = button_about(button_mask);
        break;
        /* ------------ STATE VERSION -------------- */
    case STATE_VERSION:
        button_version(button_mask);
        return;
        /* ------------ STATE MORE INFO -------------- */
    case STATE_MORE_INFO:
        array_sz = button_more_info(button_mask);
        break;
        /* ------------ STATE BIP PATH -------------- */
    case STATE_BIP_PATH:
        array_sz = button_bip_path(button_mask);
        break;
        /* ------------ STATE DISPLAY_ADDRESS -------------- */
    case STATE_DISP_ADDR:
        array_sz = button_disp_addr(button_mask);
        break;
        /* ------------ STATE DISPLAY CHECKSUM -------------- */
    case STATE_DISP_ADDR_CHK:
        button_disp_addr_chk(button_mask);
        return;
        /* ------------ STATE MENU_TX_ADDRESS -------------- */
    case STATE_TX_ADDR:
        array_sz = button_tx_addr(button_mask);
        break;
        /* ------------ PROMPT TX INFO *DYNAMIC-MENU* -------------- */
    case STATE_PROMPT_TX:
        button_prompt_tx(button_mask);
        return;
    case STATE_IGNORE:
        return;
        /* ------------ DEFAULT -------------- */
    default: // this would be an unkown state/error
        ui_state.menu_idx = 0;
        return;
    }

    // incr/decr menu index
    if (array_sz >= 0)
        button_handle_menu_idx(button_mask, array_sz);
}

/* ----------------------------------------------------
 ------------------------------------------------------
 Default display options per state
 ------------------------------------------------------
 --------------------------------------------------- */
static void nanos_build_display()
{
    switch (ui_state.state) {
        /* ------------ MAIN MENU -------------- */
    case STATE_MAIN_MENU:
        display_main_menu();
        break;
        /* ------------ ABOUT -------------- */
    case STATE_ABOUT:
        display_about();
        break;
        /* ------------ VERSION -------------- */
    case STATE_VERSION:
        display_version();
        break;
        /* ------------ MORE INFO -------------- */
    case STATE_MORE_INFO:
        display_more_info();
        break;
        /* ------------ BIP PATH ------------ */
    case STATE_BIP_PATH:
        display_bip_path();
        break;
        /* ------------ DISPLAY TX ADDRESS -------------- */
    case STATE_TX_ADDR:
        display_tx_addr();
        break;
        /* ------------ DISPLAY ADDRESS MENU -------------- */
    case STATE_DISP_ADDR:
        display_addr();
        break;
        /* ------------ DISPLAY ADDRESS CHECKSUM -------------- */
    case STATE_DISP_ADDR_CHK:
        display_addr_chk();
        break;
        /* ------------ PROMPT TX *DNYMANIC-MENU -------------- */
    case STATE_PROMPT_TX:
        display_prompt_tx();
        break;
        /* ------------ IGNORE STATE -------------- */
    case STATE_IGNORE:
        return;
        /* ------------ UNKNOWN STATE -------------- */
    default:
        display_unknown_state();
        break;
    }
}

/* ----------------------------------------------------
 ------------------------------------------------------
 ------- MAIN BUTTON LOGIC -------
 Every button press calls transition_state
 ------------------------------------------------------
 --------------------------------------------------- */
static void nanos_transition_state(unsigned int button_mask)
{
    uint8_t translated_mask = nanos_translate_mask(button_mask);

    // make sure we only transition on valid button presses
    if (translated_mask == BUTTON_BAD)
        return;

    nanos_handle_button(translated_mask);

    nanos_build_display();

    if (ui_state.state == STATE_EXIT)
        // Go back to the dashboard
        os_sched_exit(0);

    x_get_screen();
    // render new display
    nanos_render();
}

#endif // TARGET_NANOX
