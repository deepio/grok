//***************************************************************************/
// This software is released under the 2-Clause BSD license, included
// below.
//
// Copyright (c) 2019, Aous Naman
// Copyright (c) 2019, Kakadu Software Pty Ltd, Australia
// Copyright (c) 2019, The University of New South Wales, Australia
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//***************************************************************************/
// This file is part of the OpenJPH software implementation.
// File: ojph_expand.cpp
// Author: Aous Naman
// Date: 28 August 2019
//***************************************************************************/


#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
using namespace std;

namespace grk {

struct param_qcd
{
public:
  param_qcd() : Lqcd(0),
  	  	  	  Sqcd(0),
			  num_decomps(0),
			  base_delta(-1.0f)
  {
  }

  void set_delta(float delta) { base_delta = delta; }
  void set_rev_quant(int bit_depth, bool is_employing_color_transform);
  void set_irrev_quant();

  void check_validity(int decomps,bool is_reversible,
		  	  	  int max_bit_depth, bool color_transform, bool is_signed )
  {
	num_decomps =decomps;
	if (is_reversible)
	{
	  set_rev_quant(max_bit_depth, color_transform);
	}
	else
	{
	  if (base_delta == -1.0f)
		base_delta = 1.0f /
		  (float)(1 << (max_bit_depth + is_signed));
	  set_irrev_quant();
	 }
  }

  int get_num_guard_bits() const;
  int get_MAGBp() const;
  int get_Kmax(int resolution, int subband) const;
  int rev_get_num_bits(int resolution, int subband) const;
  float irrev_get_delta(int resolution, int subband) const;

private:
  uint16_t Lqcd;
  uint8_t Sqcd;
  union
  {
	uint8_t u8_SPqcd[97];
	uint16_t u16_SPqcd[97];
  };
  int num_decomps;
  float base_delta;
};

}


