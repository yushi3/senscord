/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_PYTHON_SRC_PYTHON_LIMITED_H_
#define LIB_PYTHON_SRC_PYTHON_LIMITED_H_

#define Py_LIMITED_API 0x03060000

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

#endif  // LIB_PYTHON_SRC_PYTHON_LIMITED_H_
