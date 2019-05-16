# Adventure
RPG turn based game
This is a personal project I've been working on since the start of April 2, 2018. 

Adventure is a 3D top down turn based game where friends can play out their hero's stories. 
It's based upon the large Franchise of DnD and influenced by the popular website Roll20. 

With the power of Unreal Engine and Steam's API, maps that are dynamically created by the player's can be played
and shared across the comunity to offer experiences unique to each's stories. (Using UE4 v.4.21)

## Features:
- Connect and play with friends using Steam
- Creation of dynamic maps which push the limits of Unreal Engine's potential for optimization
- Interactable play sessions with changing variables such as real time player movement, dynamically triggered events, and features like fog of war
- Fies for such areas that can be shared across the community to allow for different experiences.
- Responsive system that doesn't hinder play experience

## How To Play:
###### Before you play, make sure you are running steam on the computer to allow the game to access its networking features.
- To play the game, you first have to either build the game using UE4 version 4.21 or run in Standalone version only!
  (Steamworks does not run correctly in PIE)
- There are three options on the main menu: Host, Join Session, and Game Builder.
- To leave a mode, use the ESC key to bring up the menu.

### Game Builder:
- Before anyone can play, a map has to be created. To do this, choose the Game Builder option. Here you have the option to edit
and create maps for a game. (Day time controls do not currently work)
- There are many options to modify a game map. The currently working options are Terrain and Objects.
- Using the Terrain editor, you are able to increase/decrease elevation and change textures. To select an area in this mode, 
click and drag an area using your cursor.
- Objects can be added or deleted. Any model is consitered an object. When placing an object you can use the -,+ keys to rotate the model before placing it.
- You may click and drag an area to select objects and remove them.
- In the settings, there are two options: "snap movement to grid" (allows the cursor to move in grid space) and "Allow object to block placement" (you may place object on top of other objects in this mode)
- Using the Inventory Items editor, you can add, remove and modify weapons/consumables that will be used in your game.
- This mode allows you to select visuals to represent them when a player's "Pawn" takes it out of its inventory.
- By Default, when playing the player will pull out a sword called "Basic Sword". (Testing purpouses)

### Host:
- To play as DM, you can Host a game. To host a game, you must provide a Session name which will be publicly announced to players looking to join a game. (make private is currently disabled)
- When the game is loaded up, you will have the choice to select a map. When the host selects a map, all connected players as well as any incoming players will begin download a copy of it. The download may take a while depending on the size of the loaded map.
- On joinin a map, fog of war is turned on.
- The host has the ability to place new pawns into the map. To do this, left-click on the grid at any point and select "Spawn pawn here". As soon as the pawn is created, the fog of war around the player clears out.
- To assign a pawn to a specific connected player, left-click the pawn and select "Inspect". When inspecting the pawn, an icon in the shape of a pawn will apear in the bottom right. Click the icon to show the menu and select the owner from a drop down menu.
- To quickly switch between inspect mode and overview mode, press the "C" key.
- To zoom in, use the scroll wheel.
- To remove a pawn, left-click on the pawn and select "Remove pawn"
- When in inspect mode, you will have the option to attack appear in the context menu (left-click menu).
- On the left of the screen, there are three sub menus you can open up: World Settings, Items menu, and Pawn Menu. Currently only the Items menu works. (This is the same menu as the one in Game Builder)

### Join Session
- If a public game is available, they will be posted here.
- To join a game, click on the session name.
- The map may not load at the very beginning because the host might not have selected a map yet. When the host does, a loading screen will be presented to signal the progress of the map download.
- As soon as the map is loaded it will be covered in fog of war. As soon as the DM places a pawn, the fog of war in the area around the pawn will disperse.
- To control a pawn, the host will need to give control of an active pawn. Once this is done, all controls previously locked will be made available.
- When moving a pawn, the game will automatically determine the shortest path to a destination and move accordingly. If no path is available, it will remain stationary. The more you move, the more of the map will be revealed.
- If disconnected, all previous permissions will persist when rejoining a session.


## TODO:
- Add item templates to the game wich allow for custom pickups in each session.
- Better design tools to allow for more unique maps
- Attacks still need to replicate over the network
- Maps need to be packed more efficiently to reduce download times and network transfer ammounts
- Thematic UI for gameplay, main menu, and game-builder.
- UI to organize all the available assets by type to allow items to be found more easily
- More models = more possibilities
- Optimization of all features to allow Adventure to run on lower end computers

###### The specs for running Adventure aren't too high as it's been tested on several laptops like Window's Surface but it is recommended to have around an Intel I7 core processor, GTX 960-970, and around 8 gb of ram.

###### Some files may not load due to Github's file size limit. The project should still be able to be built and ran.
