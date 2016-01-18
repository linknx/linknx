/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2006 Martin K�gler <mkoegler@auto.tuwien.ac.at>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "threads.h"

void *
Thread::ThreadWrapper (void *arg)
{
    Thread *t = (Thread *) arg;
    t->Run (&t->should_stop);
    if (!t->joinable)
        t->tid = 0;
    if (t->autodel)
        delete t;
    pth_exit (0);
    return 0;
}

Thread::Thread (int Priority, Runable * o, THREADENTRY t)
{
    autodel = false;
    joinable = true;
    obj = o;
    entry = t;
    pth_sem_init (&should_stop);
    prio = Priority;
    tid = 0;
}

Thread::~Thread ()
{
    Stop ();
}

void
Thread::Stop ()
{
    if (!tid)
        return;
    pth_sem_inc (&should_stop, TRUE);

    if (!joinable || pth_join (tid, 0))
        tid = 0;
}

void
Thread::StopDelete ()
{
    autodel = true;
    pth_sem_inc (&should_stop, FALSE);
    if (!tid)
        return;
    pth_attr_t at = pth_attr_of (tid);
    pth_attr_set (at, PTH_ATTR_JOINABLE, FALSE);
    pth_attr_destroy (at);
}

void
Thread::Start (bool detach)
{
    if (joinable && tid)
    {
        pth_attr_t a = pth_attr_of (tid);
        int state;
        pth_attr_get (a, PTH_ATTR_STATE, &state);
        pth_attr_destroy (a);
        if (state != PTH_STATE_DEAD)
            return;
        Stop ();
    }
    pth_sem_init (&should_stop);
    pth_attr_t attr = pth_attr_new ();
    pth_attr_set (attr, PTH_ATTR_PRIO, prio);
    joinable = !detach;
    if (detach)
        pth_attr_set (attr, PTH_ATTR_JOINABLE, FALSE);
    tid = pth_spawn (attr, &ThreadWrapper, this);
    pth_attr_destroy (attr);
}

void
Thread::Run (pth_sem_t * stop)
{
    (obj->*entry) (stop);
}

bool Thread::isRunning ()
{
    return tid == pth_self();
}

bool Thread::isFinished ()
{
    if (tid ==0)
        return true;
    pth_attr_t a = pth_attr_of (tid);
    int state;
    pth_attr_get (a, PTH_ATTR_STATE, &state);
    pth_attr_destroy (a);
    return (state == PTH_STATE_DEAD);
}

