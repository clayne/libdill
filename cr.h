/*

  Copyright (c) 2016 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef DILL_CR_INCLUDED
#define DILL_CR_INCLUDED

#include <setjmp.h>
#include <stdint.h>

#include "list.h"
#include "libdill.h"
#include "slist.h"

struct dill_cr;

struct dill_clause {
    /* The coroutine that owns this clause. */
    struct dill_cr *cr;
    /* List of clauses coroutine is waiting for. See dill_cr::clauses. */
    struct dill_slist_item item;
    /* Number to return from dill_wait() if this clause triggers. */
    int id;
    /* These two fields are completely opaque to the coroutine. They are meant
       to be used by endpoints. The only thing coroutine does with it is, just
       before dill_wait() exits, it removes 'epitem' from 'eplist' for each
       clause. 'eplist' can be NULL in which case removal doesn't happen. */
    struct dill_list_item epitem;
    struct dill_list *eplist;
};

/* When dill_wait() is called next time, the coroutine will wait
   (among other clauses) for this clause. 'id' must not be negative.
   'eplist' is the list to add the clause to (can be NULL). 'epitem' is the
   item in the list to insert the clause before. If NULL it will be inserted
   at the end of the list. Call to dill_wait() will remove the clause from
   the list. */
void dill_waitfor(struct dill_clause *cl, int id,
    struct dill_list *eplist, struct dill_list_item *before);

/* Suspend running coroutine. Move to executing different coroutines.
   The coroutine will be resumed once one of the clauses previously added by
   dill_waitfor() is triggered or dill_cancel() is called. Once that happens
   all the clauses, whether triggered or not, will be canceled. Function returns
   ID of the triggered clause or -1 in case dill_cancel() was called. In either
   case it sets errno to the value supplied in dill_trigger/dill_cancel. */
int dill_wait(void);

/* Following two functions schedule previously suspended coroutine for
   execution. Keep in mind that they don't immediately run it, just put it into
   the queue of ready coroutines. First one will cause dill_wait() return
   the id supplied in dill_waitfor(). Second one will cause it to return -1.
   'err' is used to set errno in the resumed coroutine. */
void dill_trigger(struct dill_clause *cl, int err);
void dill_cancel(struct dill_cr *cr, int err);

/* Starts canceling the main coroutine. */
void dill_shutdown(void);

/* Returns 0 if blocking functions are allowed.
   Returns -1 and sets errno to ECANCELED otherwise. */
int dill_canblock(void);

/* Makes the current coroutine the main coroutine of the child process. */
void dill_cr_postfork(void);

/* TODO: Can we get rid of this function? */
int dill_no_blocking2(int val);

#endif

