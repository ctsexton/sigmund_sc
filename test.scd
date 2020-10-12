
(
b = Buffer.alloc(s, 20, 3);
)

(
{
	// var input = Mix.ar(SinOsc.ar([300, 500, 800]));
	var input = SoundIn.ar(0);
	var sig, freq, amp, action, rands;
	Sigmund.ar(input, b, 3);
	freq = 20.collect({ |i| Index.kr(b, i * 3) });
	amp = 20.collect({ |i| Index.kr(b, i * 3 + 1) });
	action = 20.collect({ |i| Index.kr(b, i * 3 + 2) });

	freq.poll;
	rands = Array.rand2(20, 100) * 0.1;

	freq = freq.cpsmidi;
	freq = freq.midicps;

	sig = SinOsc.ar(freq.varlag(0.01), mul: amp.clip(0, 1));
	sig = Mix.ar(sig);
	Out.ar(0, sig!2);


}.play(s)
)

