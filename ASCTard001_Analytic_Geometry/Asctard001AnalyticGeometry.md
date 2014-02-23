ASCTard001 - Analytic Geometry 
===============================

Analytic Geometry records and plays back 64 steps of CV. The step played back from the pattern recorded is determined by the inputs of a2 and a3.

Our 64 values are referenced as a grid where each of the blank spaces holds a value

  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
7 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
6 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
5 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
4 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
3 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
2 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
1 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
0 x   x   x   x   x   x   x   x   x
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    0   1   2   3   4   5   6   7

(note: the above looks totally screwed in markdown, look at it in a text editor)

when the program gets a gate high in it looks at the current inputs of a2 and a3 and uses them to determine the x/y position that it should play.


###Knobs:

##A0 - Mode knob. 
		Record Mode
			- First 3rd of the dial (roughly 7 to 11 o'clock)
			- In this mode the ardcore receives cv at input a2 when gate is high
		Playback Mode + value injection
			- Second 3rd of the dial (11 to 2)
			- In this mode the ardcore, on a gate, plays back the sequenced based on the input but injects a value from the original pattern when a2 and a3 are at 0,
			- This means that if knobs a2 and a3 are at 0 it plays back the original pattern
		Playback Mode
			- Third 3rd of dial (2 to 5)
			- Plays back the sequence based on the incoming cv to determine co-ordinates
			- if there is no cv in then the note will stay the same

