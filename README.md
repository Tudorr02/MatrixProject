
# Impossible Road Game ☠️

## Description
"Impossible Road" is an engaging Arduino-based game that features an LCD display and joystick controls. Players navigate through maze-like maps, avoiding enemies, and collecting keys across various levels.
 The game features:

- Dynamic Levels: Each level comes with a different layout of walls, enemies, and keys.
- Enemies and Attacks: Enemies are placed throughout the map, each capable of attacking in a specific pattern.
- Key Collection: Players must collect keys scattered across the map to unlock subsequent levels.
- Player Lives: The player starts with a set number of lives, which are deleted by enemy attacks.
- High Score Tracking: The game records high scores, adding a competitive edge.<br>
  <br> <img src="https://github.com/Tudorr02/MatrixProject/assets/92024989/fb9fa62b-bd99-4d56-a800-50f1c4d4c7b9" width="700" height="500">

## How to Play
- **Movement** : Use the analog joystick to navigate through the map. Move up, down, left, or right to avoid walls and enemies.
- **Collecting** Keys: Navigate to key locations to collect them. Keys are necessary to complete levels.
- **Avoiding Enemies** : Stay clear of enemy attack patterns. Getting hit by an enemy attack will cost a life.
- **Completing Levels** : Collect the minimum required keys and press the joystick button to complete a level.
- **Losing Lives** : If all lives are lost, the level is failed. The game will end, and you can restart or view high scores.
- **Advancing Levels** : Upon completing a level, the next one is unlocked. Each level increases in difficulty

## Menu Structure 
### Main Menu

- **Start Game**: Leads to level selection.
- **High Score**: Displays the top scores.
- **Settings**: Adjust game settings.
- **About**: Information about the game and credits.
- **How To Play**: Gameplay instructions.

### Start Game Submenu

- **Level 1**: Begin at the initial level.
- **Level 2**: Unlocks after Level 1 completion.
- **Level 3**: Available after completing Level 2.
- **Exit**: Return to the Main Menu.

### Settings Submenu

- **Enter Name**: Set the player's in-game name.
- **LCD Brightness**: Adjust the LCD's brightness.
- **Matrix Brightness**: Set LED matrix brightness.
- **Sounds**: Toggle sound effects.
- **AlertKey**: Enable/disable alert LED.
- **Reset Scores**: Clear high scores.
- **Exit**: Back to Main Menu.

### Detailed Menu Options

- **Start Game**: Select the desired level to play, with each level offering unique challenges and layouts.
- **High Score**: Review the top achievements in the game, including player names and scores.
- **Settings**: Fine-tune the game experience, including visual and auditory aspects.
- **About**: Learn about the game's development and the creator.
- **How To Play**: Understand the basics of gameplay, controls, and objectives.
- **Enter Name**: Personalize the gaming experience with a custom player name.
- **LCD Brightness & Matrix Brightness**: Adjust these settings for optimal viewing comfort.
- **Sounds**: Engage with the game's audio aspect for a more immersive experience.
- **AlertKey**: Utilize the alert LED for gameplay cues.
- **Reset Scores**: Start anew by clearing the high score slate.


## Components Used
1. **Arduino Board**: The central processing unit of the game.
2. **Liquid Crystal Display (LCD)**: Displays game stats, levels, and menus.
3. **LED Matrix**: For dynamic visual game elements.
4. **Analog Joystick**: For player movement and menu navigation.
5. **EEPROM**: To store game settings and high scores.
6. **Buzzer**: Used for sound effects in the game.
7. **Custom Characters**: Enhance game visuals and clarity.
8. **LEDs and Resistors**: Used for alert indications and visual effects.

## About 

<img src="https://media.giphy.com/media/XYlK99u8oOGic/giphy.gif" width="100" height="100" />
At its core, "Impossible Road" is born out of a passion for matrix manipulation and a desire to push the boundaries of my coding skills. This game reflects a personal challenge I set for myself - to create an engaging, compact shooter game that's both fun and intellectually stimulating.

Here is a presentation video : https://youtu.be/tBP2JJA66c8
