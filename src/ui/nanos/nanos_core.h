#ifndef NANOS_CORE_H
#define NANOS_CORE_H

#include <stdint.h>
#include "nanos_types.h"

void nanos_set_screen(UI_SCREENS s);
void nanos_render(void);
void nanos_ctx_initialize(void);
void nanos_display_main_menu(void);
void nanos_ui_init(void);
void nanos_display_getting_addr(void);
void nanos_display_validating(void);
void nanos_display_recv(void);
void nanos_display_signing(void);
void nanos_display_address(const unsigned char *addr_bytes);
void nanos_sign_tx(void);
void nanos_ui_reset(void);
void nanos_ui_restore(void);

void ui_timeout_tick(void);
void nanos_handle_button(uint8_t button_mask);
void nanos_build_display(void);
void nanos_transition_state(unsigned int button_mask);

#endif // NANOS_CORE_H
