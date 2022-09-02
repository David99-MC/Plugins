# This plugin can be added to any Unreal projects to connect players across the Internet with matchmaking session!

# Overview of this plugin's structure:

# There are two main classes:
  a. MultiPlayerSessionSubsystem: This class is responsible for handling all of the multiplayer session-related functionality.
  b. Menu: This class handles the button functionality namely HostButton and JoinButton, which is reflected in the Editor's widget.

# The relationship between the classes and the OnlineSubsystem:
  There is only one way dependency between the MultiPlayerSubsystem and the OnlineSubsystem. I achieve this by creating delegates in the MultiPlayerSubsystem class and have the Menu class bind its functions to it, and then I simply broadcast those delegates in the MultiPlayerSubsystem class when needed. By doing this, I increase the flexibility of this plugin, for example, if I want to create another menu in the future, I only need to follow the same pattern: bind the new menu class's functions to the newly created delegates and have the other class broadcast them.
