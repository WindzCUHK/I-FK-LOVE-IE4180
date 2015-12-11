/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: CgiUtils.h,v 1.17 2014/04/23 20:55:03 sebdiaz Exp $
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth <sbooth@gnu.org>
 *                       2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
 *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#ifndef _CGIUTILS_H_
#define _CGIUTILS_H_ 1

/*! \file CgiUtils.h
 * \brief A collection of utility functions.
 *
 * These utility functions are used internally by cgicc to
 * decode posted form data, and to read/write from streams.
 */

#include <string>
#include <cctype>

namespace cgicc {
  
  /*!
   * \brief Convert encoded characters in form data to normal ASCII. 
   *
   * For example, "%21" is converted to '!' and '+' is converted to a space.
   * Normally, this is called internally to decode the query string or post 
   * data.
   * \param src The src string containing the encoded characters

   * \return The converted string
   */
	std::string
  form_urldecode(const std::string& src);

  /*!
   * \brief Convert an ASCII string to a URL-safe string.
   *
   * For example, '!' is converted to "%21" and ' ' is converted to '+'.
   * \param src The src string containing the characters to encode
   * \return The converted string
   */
	std::string
  form_urlencode(const std::string& src);
  

  /*!
   * \brief Convert an ASCII character to its hexadecimal equivalent.
   *
   * For example, after the call
   * \code
   * string s = charToHex(':');
   * \endcode
   * \c s will have a value of "3A".
   * Normally, this is called internally to encode characters by
   * escapeString.
   * \param c The character to encode
   * \return A string representing the hexadecimal value of c
   */
	std::string
  charToHex(char c);
  
  /*!
   * \brief Convert a hex-encoded character to its ASCII equivalent.
   *
   * For example, after the call
   * \code
   * char c = hexToChar('2', '1');
   * \endcode
   * \c c will have a value of '!'.
   * Normally, this is called internally to decode encoded characters in
   * the query string or post data.
   * \param first The first hex digit
   * \param second The second hex digit
   * \return The ASCII character
   */
	char
  hexToChar(char first,
	    char second);
  
} // namespace cgicc

#endif /* ! _CGIUTILS_H_ */
