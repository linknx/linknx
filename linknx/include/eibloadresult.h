/*
    EIBD client library
    Copyright (C) 2005-2006 Martin Kögler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License, 
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any 
    restriction coming from the use of this file. (The General Public 
    License restrictions do apply in other respects; for example, they 
    cover modification of the file, and distribution when not linked into 
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef EIB_LOAD_RESULT_H
#define EIB_LOAD_RESULT_H

typedef enum
{
  IMG_UNKNOWN_ERROR = 0,
  IMG_UNRECOG_FORMAT,
  IMG_INVALID_FORMAT,
  IMG_NO_BCUTYPE,
  IMG_UNKNOWN_BCUTYPE,
  IMG_NO_CODE,
  IMG_NO_SIZE,
  IMG_LODATA_OVERFLOW,
  IMG_HIDATA_OVERFLOW,
  IMG_TEXT_OVERFLOW,
  IMG_NO_ADDRESS,
  IMG_WRONG_SIZE,
  IMG_IMAGE_LOADABLE,
  IMG_NO_DEVICE_CONNECTION,
  IMG_MASK_READ_FAILED,
  IMG_WRONG_MASK_VERSION,
  IMG_CLEAR_ERROR,
  IMG_RESET_ADDR_TAB,
  IMG_LOAD_HEADER,
  IMG_LOAD_MAIN,
  IMG_ZERO_RAM,
  IMG_FINALIZE_ADDR_TAB,
  IMG_PREPARE_RUN,
  IMG_RESTART,
  IMG_LOADED,
  IMG_NO_START,
  IMG_WRONG_ADDRTAB,
  IMG_ADDRTAB_OVERFLOW,
  IMG_OVERLAP_ASSOCTAB,
  IMG_OVERLAP_TEXT,
  IMG_NEGATIV_TEXT_SIZE,
  IMG_OVERLAP_PARAM,
  IMG_OVERLAP_EEPROM,
  IMG_OBJTAB_OVERFLOW,
  IMG_WRONG_LOADCTL,
  IMG_UNLOAD_ADDR,
  IMG_UNLOAD_ASSOC,
  IMG_UNLOAD_PROG,
  IMG_LOAD_ADDR,
  IMG_WRITE_ADDR,
  IMG_SET_ADDR,
  IMG_FINISH_ADDR,
  IMG_LOAD_ASSOC,
  IMG_WRITE_ASSOC,
  IMG_SET_ASSOC,
  IMG_FINISH_ASSOC,
  IMG_LOAD_PROG,
  IMG_ALLOC_LORAM,
  IMG_ALLOC_HIRAM,
  IMG_ALLOC_INIT,
  IMG_ALLOC_RO,
  IMG_ALLOC_EEPROM,
  IMG_ALLOC_PARAM,
  IMG_SET_PROG,
  IMG_SET_TASK_PTR,
  IMG_SET_OBJ,
  IMG_SET_TASK2,
  IMG_FINISH_PROC,
  IMG_WRONG_CHECKLIM,
  IMG_INVALID_KEY,
  IMG_AUTHORIZATION_FAILED,
  IMG_KEY_WRITE,
} BCU_LOAD_RESULT;

#endif
