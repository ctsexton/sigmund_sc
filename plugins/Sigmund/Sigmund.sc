Sigmund : MultiOutUGen {
	*ar { |input, bufnum, numTracks|
		/* TODO */
		^this.multiNew('audio', bufnum, numTracks, input);
	}
	checkInputs {
		/* TODO */
		^this.checkValidInputs;
	}
}
