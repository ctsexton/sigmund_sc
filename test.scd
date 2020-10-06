
(
s = Server.remote(\debug);
s.serverRunning;
b = Buffer.alloc(s, 20, 3);
)

(
{
	var input = SinOsc.ar();
	var sig, freq, amp, action;
	Sigmund.ar(input, b, 3);
	freq = 20.collect({ |i| Index.kr(b, i * 3) });
	amp = 20.collect({ |i| Index.kr(b, i * 3 + 1) });
	action = 20.collect({ |i| Index.kr(b, i * 3 + 2) });

	freq.poll;

}.play(s)
)

Extensions