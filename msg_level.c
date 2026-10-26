/* msg_level.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Message Level Handling                                                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "msg_level.h"

unsigned msg_level       = e_msg_level_normal;
ShortInt msg_quiet_depth = 0;

static as_cmd_result_t cmd_msg_level_quiet(Boolean negate, char const* p_arg) {
    UNUSED(p_arg);

    if (negate && msg_quiet_depth > 0) {
        msg_quiet_depth--;
    } else if (!negate && msg_quiet_depth < 2) {
        msg_quiet_depth++;
    }
    msg_level = negate ? e_msg_level_normal : e_msg_level_quiet;
    return e_cmd_ok;
}

static as_cmd_result_t cmd_msg_level_verbose(Boolean negate, char const* p_arg) {
    UNUSED(p_arg);

    if (negate) {
        if (msg_level > 0) {
            msg_level--;
        }
    } else {
        msg_level++;
    }
    return e_cmd_ok;
}

static as_cmd_rec_t const cmds_msg_level[] = {
        {    "q",   cmd_msg_level_quiet},
        {"quiet",   cmd_msg_level_quiet},
        {    "v", cmd_msg_level_verbose}
};

static void reset_options(void) {
    msg_level = e_msg_level_normal;
}

void msg_level_init(void) {
    as_cmd_register_reset(reset_options);
    msg_level = e_msg_level_normal;
    as_cmd_register(cmds_msg_level, as_array_size(cmds_msg_level));
}
