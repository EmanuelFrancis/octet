# Introduction to Programming - REPORT

Emanuel Francis - 33442263 - MA Computer Games Art and Design

  - Type some Markdown on the left
  - See HTML in the right
  - Magic


My adaptations for the Invaderers game has not been ground-breaking by any means, however I have 
implemented certain fundamental core features of which i feel have helped my understanding of programming 
structure and logic a great deal, and have given me enough to allow me to continue to develop this game and improve my C++ 
skills in my own time.

### Sprite changes: *change spaceship sprite*
<img src="https://raw.githubusercontent.com/EmanuelFrancis/octet/master/octet/assets/invaderers/missile.gif" width="10px"><br />
Using the same filetype and dimensions I found a license free spaceship sprite and replaced the current spaceship sprite from the invaderers/assets folder using exactly the same name and extension so that my new sprite would be read instead.


### Sound changes: *change missile sounds*
In the same manner as changing the sprites, I found a laser gun .wav sound file with a similar bitrate to the bang and woosh sound files and replaced them respectively using the exact same file names and extensions.

### Explosion sprite added: *timed to display for 5 frames*
When an enemy is hit, an explosion sprite wil display. In order to add an additional sprite to the game I had to declare it within the enum list, find a license free explosion sprite and load the sprite in the same way as the other other sprites were loaded. Within the 'animate the missiles' function I created a counter to count a set number of frames, of which I chose 5, which would be controlled by a boolean 'a switch'. When an invaderer is hit, the explosion sprite is shown relative the that invaderer, the counter is turned on and an if statement is used to move the sprite out of view, turn off and reset the counter after 5 frames, simulating a basic explosion.

### Missile Power-ups: *2 power-up levels added*
When 4 invaderers have been killed, the missile sprite changes colour from green to blue to indicate a weapon power-up. When 8 invaderers have been killed the missile sprite changes again to brown and 3 missiles are fired at once as the second power-up. If you are hit however, you will go down a power-up level. To do this I created an array storing the power-ups, and a counter that goes to the next value in the array each time a powerup is activated.

### Load a .csv file: *invaderer formations*
I have disregarded the current invaderers formation setup and used .csv files to form the formations of the invaderers. Using a grid of 6x6 where the values within the cells if "1" represent an invaderer and if "0" represent no invaderer. This method allows me to apply a visual representation within the .csv grid to plot out invaderer formations. The .csv is loaded using Andy Thomasons .csv example code and the invaderers are displayed using the same sprite method set with a padding for the sides. 

### Load next level: *next level loaded once all invaderers are dead*
Once all invaderers have been destroyed a function is called to load the next level. This function contains a counter which adds one to the current level allowing the read_file function to read the name of the .csv file as a string in a sequencial order. For example, when the file 'inv_formation1.csv' (level 1) has been completed, it will automatically search for the file 'inv_formation2.csv' in order to load the second level. A maximum level value has been set so that if the counter goes past the set value which I have set as 2, as there are 2 levels, then the game will end.

### Custom shader created: *change the background of the game*
The background colour of the game has been changed to display a black to blue gradient effect. I did this by creating my own shader using the current shader as a template. By going through the code of the current template disregarding lines i felt where unnecessary for my needs such as mapping to uvs and creating a blue gradient within the fragment shader section. I then loaded the shader within the invaderers code by using the same method and replacing the current shader that was being used. This task required quite abit of help from Mircea Catana. 

### Reference and help recieved
Refererences used where Andy Thomasons code examples, many online forums and C++ for Dummies reference guide.
I would especially like to thank the following people for their patience with my questions, helpful explanations, code examples and troubleshooting.

* Mircea Catana
* Alejandro Saura Villanueva
* Raul Araujo
* Mohammad Ali