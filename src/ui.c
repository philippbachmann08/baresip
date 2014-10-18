/**
 * @file ui.c  User Interface
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include "core.h"


static struct list uil;  /**< List of UIs (struct ui) */
static struct cmd_ctx *uictx;


static void ui_handler(char key, struct re_printf *pf)
{
	(void)cmd_process(&uictx, key, pf);
}


static int stdout_handler(const char *p, size_t size, void *arg)
{
	(void)arg;

	if (1 != fwrite(p, size, 1, stdout))
		return ENOMEM;

	return 0;
}


/**
 * Register a new User-Interface (UI) module
 *
 * @param ui The User-Interface (UI) module to register
 */
void ui_register(struct ui *ui)
{
	if (!ui)
		return;

	list_append(&uil, &ui->le, ui);

	debug("ui: %s\n", ui->name);
}


/**
 * Un-register a User-Interface (UI) module
 *
 * @param ui The User-Interface (UI) module to un-register
 */
void ui_unregister(struct ui *ui)
{
	if (!ui)
		return;

	list_unlink(&ui->le);
}


/**
 * Send input to the UI subsystem
 *
 * @param key Input character
 */
void ui_input(char key)
{
	static struct re_printf pf_stdout = {stdout_handler, NULL};

	ui_handler(key, &pf_stdout);
}


/**
 * Send an input key to the UI subsystem, with a print function for response
 *
 * @param key Input character
 * @param pf  Print function for the response
 */
void ui_input_key(char key, struct re_printf *pf)
{
	ui_handler(key, pf);
}


/**
 * Send an input string to the UI subsystem
 *
 * @param str Input string
 */
void ui_input_str(const char *str)
{
	struct re_printf pf;
	struct pl pl;

	if (!str)
		return;

	pf.vph = stdout_handler;
	pf.arg = NULL;

	pl_set_str(&pl, str);

	(void)ui_input_pl(&pf, &pl);
}


int ui_input_pl(struct re_printf *pf, const struct pl *pl)
{
	struct cmd_ctx *ctx = NULL;
	size_t i;
	int err = 0;

	if (!pf || !pl)
		return EINVAL;

	for (i=0; i<pl->l; i++) {
		err |= cmd_process(&ctx, pl->p[i], pf);
	}

	if (pl->l > 1 && ctx)
		err |= cmd_process(&ctx, '\n', pf);

	return err;
}


/**
 * Send output to all modules registered in the UI subsystem
 *
 * @param str Output string
 */
void ui_output(const char *str)
{
	struct le *le;

	for (le = uil.head; le; le = le->next) {
		const struct ui *ui = le->data;

		if (ui->outputh)
			ui->outputh(str);
	}
}


/**
 * Reset the state of the UI subsystem, free resources
 */
void ui_reset(void)
{
	uictx = mem_deref(uictx);
}
