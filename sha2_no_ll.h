#ifndef _PPC64_LE_SHA2_NO_LL_H_
#define _PPC64_LE_SHA2_NO_LL_H_

base_type rotate_right(base_type num, base_type bits) {
	return ((num >> bits) | (num << (base_type_size*8 - bits)));
}
base_type calc_S0(base_type a) {
	base_type tmp1, tmp2, tmp3;
	tmp1 = rotate_right(a, S0_args[0]);
	tmp2 = rotate_right(a, S0_args[1]);
	tmp3 = rotate_right(a, S0_args[2]);
	return tmp1 ^ tmp2 ^ tmp3;
}
base_type calc_S1(base_type e) {
	base_type tmp1, tmp2, tmp3;
	tmp1 = rotate_right(e, S1_args[0]);
	tmp2 = rotate_right(e, S1_args[1]);
	tmp3 = rotate_right(e, S1_args[2]);
	return tmp1 ^ tmp2 ^ tmp3;
}
base_type calc_s0(base_type a) {
	base_type tmp1, tmp2, tmp3;
	tmp1 = rotate_right(a, s0_args[0]);
	tmp2 = rotate_right(a, s0_args[1]);
	tmp3 = a >> s0_args[2];
	return tmp1 ^ tmp2 ^ tmp3;
}
base_type calc_s1(base_type e) {
	base_type tmp1, tmp2, tmp3;
	tmp1 = rotate_right(e, s1_args[0]);
	tmp2 = rotate_right(e, s1_args[1]);
	tmp3 = e >> s1_args[2];
	return tmp1 ^ tmp2 ^ tmp3;
}

void calculate_higher_values(base_type *w) {
	for (int j = 16; j < W_SIZE; j++) {
		base_type s0,s1;
		s0 = calc_s0(w[j-15]);
		s1 = calc_s1(w[j-2]);
		w[j] = w[j-16] + s0 + w[j-7] + s1;
	}
}

void calc_compression(base_type *_h, base_type *w) {
	base_type a, b, c, d, e, f, g, h;
	a = _h[0];
	b = _h[1];
	c = _h[2];
	d = _h[3];
	e = _h[4];
	f = _h[5];
	g = _h[6];
	h = _h[7];

	for (int i = 0; i < W_SIZE; i++) {
		base_type S1, S0;
		S0 = calc_S0(a);
		S1 = calc_S1(e);
		base_type ch = calc_ch(e, f, g);
		base_type temp1 = h + S1 + ch + k[i] + w[i];
		base_type maj = calc_maj(a, b, c);
		base_type temp2 = S0 + maj;

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}
	_h[0] += a;
	_h[1] += b;
	_h[2] += c;
	_h[3] += d;
	_h[4] += e;
	_h[5] += f;
	_h[6] += g;
	_h[7] += h;
}

#endif // _PPC64_LE_SHA2_NO_LL_H_
// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
