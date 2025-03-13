# tfy4190_game

Legger inn forslag om at vi lager en melodi, for eksempel her er en chatgpt har laget. Tanken er at når vi venter 100ms på input, så kan vi endre tone, sånn at vi maks kan endre tone hvert 100'ede ms. 0 representerer ingen tone

int melody[] = {
  262, 262, 262, 0,    // C4 for 300ms (3 times 100ms) + silence (100ms)
  294, 294, 294, 0,    // D4 for 300ms + silence
  330, 330, 330, 0,    // E4 for 300ms + silence
  349, 349, 349, 0,    // F4 for 300ms + silence
  392, 392, 392, 392, 0,  // G4 for 400ms (4 times 100ms) + silence
  440, 440, 440, 440, 0,  // A4 for 400ms + silence
  494, 494, 494, 0,    // B4 for 300ms + silence
  523, 523, 523, 0,    // C5 for 300ms + silence
  523, 523, 523, 523, 0,  // C5 for 400ms + silence
  494, 494, 494, 0,    // B4 for 300ms + silence
  440, 440, 440, 0,    // A4 for 300ms + silence
  392, 392, 392, 392, 0,  // G4 for 400ms + silence
  349, 349, 349, 0,    // F4 for 300ms + silence
  330, 330, 330, 0,    // E4 for 300ms + silence
  294, 294, 294, 0,    // D4 for 300ms + silence
  262, 262, 262, 0     // C4 for 300ms + silence
};
