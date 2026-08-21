VL-Wizard
A comprehensive window to the sonic
possibilities of the VL70m
Manual Version 1.0
Section 1
Preface
As owner of the VL70m I was frustrated not being able to unlock its full power and get better sounds
out of it. The Yamaha VL software editors are out-of-date and I was at a point to sell everything. But I
knew that the VL70m is a special synth and my curiosity became bigger after I found the VL-guidelines
of Manny Fernandez. The idea of an entire new VL70m editor was born.
Building a VL editor was a very difficult journey with many pitfalls, technical complexity, lack of detailed
information and support etc. I had to start-over multiple times and I’m not joking by saying that thou-
sands of hours went into this project.. The supportive words of my beta testers and the donations
helped me to get to the end of this marathon project. Many thanks for this.
The deeper I went into the VL70m the more I got impressed on the fantastic job Yamaha has done to
built such synth. I hope that this manual encourages you to re-discover your VL-synth, build and share
great sounding patches.
Enjoy the rebirth of your VL70m,
Rudy
1
Section 2
What is it and what is it not?
The VL-Wizard is a stand-alone software. It runs separately from other music software and can not be
```
loaded from within a DAW hosting software such as (Cubase, Logic, Ableton Live, etc). The software
```
run on the latest MAC and WINDOWS operating systems. It does not run older platforms such as
WIN98, WIN XP, WIN2000, MAC POWERBOOK etc.
It is specially designed to create, modify, organize the voice patches of the VL70m and PLGVL ex-
pansion cards for the MU series, Motif ES, EX5 etc. The VL-Wizard can control up to 8 VL units in
mono, poly or multi mode. The software is not compatible with VL1 and VL7 synths or voice data.
Hundreds of parameters are well organized and documented with the VL-guidelines made by Manny
Fernandez. These guidelines show-up when you hoover over a parameter. This allows you to increase
your understanding of the purpose of many parameters and will speed-up your learning curve.
More information on VL sound design can be found in the 3rd book of professor Julius O. Smith. Click
this link to go to the CCRMA website where you will find a lot of specialized information. You might also
consider to purchase his 3rd book, click the link to go to Amazon.
A large voice library of more than 1000 voices is included. This library contains VL70m Factory,
Windlist, converted VL1, extracted visual editor voices etc. Additional to this you will also find tem-
plates for building voices from scratch.
```
The software offers 5 modules and 5 widgets (small tools).
```
MODULES WIDGETS
Voice Browser
Element Editor
Common Editor
Tuning Editor
Effect Editor
Midi phrase recorder
Midi learn
Parameter Snippet
Custom Voice storage
User Sysex
23
VL70m Effects EditorVoice Browser
Tuning Editor
Common & System Editor
Element Editor
4
User Sysex ListMidi Phraser
Midi Learn
Sysex Snippets
CST-INT Storage
Section 3
License Registration
There are 3 license modes, DEMO, BASIC and PREMIUM. The DEMO license is free but with restric-
tions such as the ability to save your changed voices, tuning etc. You require to purchase a BASIC or
PREMIUM license to be able to save your changes. The BASIC license does not allow for major up-
grades and new features. The PREMIUM license allows for lifetime free upgrades, access to all new fea-
tures and extensions and priority support.
License Registration details are send by e-mail after purchasing is completed. You require to copy the
license information into registration fields USER, AUTHOR and LICENSE KEY. The license status will
switch from DEMO to AUTHORIZED mode after successful registration.
You are allowed to install the license on multiple VL-Wizard installations under the condition that you
are the only user.
Click to open the settings screen.
Below in the settings screen you see 3 fields, USER, AUTHOR and LICENSE KEY.
Enter the your user and author name in these fields and copy/paste the license key.
Click on the text LICENSE KEY to force the validation.
Click to save the settings.
5
Figure VL-Wizard.1 License Registration
Enter Author Name
Enter User Name
Click to force the
License validation
Save the settings
Copy/Past License keyPurchase a license
License status
1
2
3
4
5
Section 4
Getting Started
Click to launch the VL-Wizard.
The initialization might take a few seconds. This is because the software is preparing voice buffers and
graphics, checking license control, midi ports etc.The first screen that appears is called the Main Con-
```
trol Center (MCC) . All features are disabled until a voice is loaded. We first need to tell the software
```
where the Voice library folder is located. This is done in the settings screen. Further in this manual we
will explain the various configuration possibilities. Now we will focus on getting you started.
Click to open the settings screen.
Navigate to the field LIBRARY and select the folder containing the VL-Voices.
```
MAC: a default location is set as part of the installation /Users/Shared/VL-Voices
```
```
WIN: the voice library is typically installed in C:\user\[USERNAME]\VL-Wizard\VL-Voices
```
Click to save the settings.
Click to close the settings screen
Click to open the voice browser screen
The Voice Browser must look like this Figure 3. You should see different libraries and when selecting a
```
library (eg: Factory) you should see the different voice categories within this library. When selecting a
```
```
category (eg: Reed) you will see all voices from the selected library that are REED types.
```
6
1
2
3
Figure VL-Wizard.2 Setting the Library Folder
Click to select the
library folder
Store the settings
4
5
6
7
Now we need to configure the midi communication with the VL70m. We assume wind controller is
hooked up to the WX-in connector of the VL70m.
Use the front panel of the VL70m to set the midi communication parameters
7
Figure VL-Wizard.3 Voice Browser layout
8
9
Set the VL70m in VOICE mode
```
Set midi-transmit channel to 1 (channel used by the WX5)
```
```
Set midi-receive channel to 1 or ALL (tone generator Rcv CH)
```
Set the Receive Sysex to ON
Set Device ID number to ALL
Go back to the VL-Wizard settings
Go to MIDI-IN CONTROLLERS
8
10
11
12
13
14
15
16
Click on the Midi-In port that is connected with your VL70m and select ON.
Default is 1: This is the midi channel used by
VL-Wizard to receive notes, controller data.
Rescan Midi-in Ports
```
IMPORTANT: Never use midi THRU when the WX5 is hooked up to the WX-in connector of
```
your VL70m. Otherwise the VL70m will get into a midi-loop and not react correctly.
Go to MIDI-OUT CONTROLLERS
Play a note on the WX5 and see if the VL-Wizard reacts.
You might want to do an extra midi communication test.
```
Click the check button (see advanced section)
```
You will see “TESTING.....”
```
Result 1 (send test): Watch the VL70m display
```
```
Result 2 (receive test): Watch the text, it should show VL70-m.
```
If the results are OK then you can proceed, if not, then recheck the connections and setup.
9
17
Default is 1: This is the midi channel
used by VL-Wizard to send midi thru
```
(notes, controller data, part data)
```
Select the Midi-Out port connected to your VL70m
Rescan Midi-Out ports
18
19
20
Default is 1, use number 2 if you
want to test your second VL70m etc.
21
22
VLWIZ icon
Your registration name
or “Hi VL-Wizard”
23
24
Section 5
Main Control Center
```
The MMC (Main Control Center) is the central place providing you access to all other features. The but-
```
tons become active once a voice is loaded.
10
Store a copy of the voice
into favorites library
Voice display name
Volume slider
Select a category
Enter comments
Midi in, thru, out led’s
Send midi reset - panic by clicking on led’s
Enter voice display name
Enter voice author name
Visual levels of the incoming midi controllers
```
Important: Midi-In port must be set to THRU
```
shows all kind of information such as sysex,
note, warnings, confirmations etc.
Enter new voice category type
Load another voice image
Shows the voice image loaded
from folder /VL-Voices/Images
Enter VL element name
show midi notes
click to select octave up/down
Switch between the loaded voices.
Will only show-up when working in UNISON
and EXPERT mode with multiple VL devices
Click to open the About window
Filename of loaded voice
Voice Category
See settings to force voice Volume
to a fixed level during loading time
Open Voice Browser
Open Element Editor
Open Tuning Editor
Open XG-Common-System Editor
Open VL70m effects Editor
Compare with original loaded voice
Open user list widget to send sysex data
Resend voice to VL unit
Open Custom/Internal storage widget
Save the loaded voice to Library and Disk
Shift -Click = Export to voice to a midi file
Open snippet widget
Open midi learn widget
Open midi phraser widget
Open VLwiz settings
Voice mode as defined in the settings
Appears only when hovering mouse
over the Main Control Center
How to load a Voice?
To load a voice you simply open the Voice Browser and select a voice in the third column. When
```
the 3rd column is highlighted (see red frame) then you may use the [Enter] key to load the selected
```
voice. You can also load a voice by [double click left mouse button] or by press the arrow
button.
What happens when loading a voice?
```
The way the VL-Wizard interacts with your VL-device(s) depends on how you configured the settings.
```
- the sysex parameters of the selected voices are transmitted to the VL-Device(s).
- the MMC will show the Display and Element Name, Category, Author, Comments and Image.
- the Midi Thru and Midi OUT LEDs will flash.
- the status section of the MCC will display : Prepare... Sending... Ready...
- the buttons on the MMC will become active.
- now you can start editing a the selected voice.
- the CURRENT VOICE buffer of the VL70M will be kept synchronized with the editing buffer of the VL-
Wizard. It is an exact mirror, so each parameter change you do in the VL-Wizard is directly transmited
to the VL70m, this allows you to hear the results of your changes in realtime.
What happens when saving a voice?
You can easily save your changes by pressing the Save button
- The combination of the DISPLAY NAME and ELEMENT NAME to construct the filename of the voice.
- A check is done if the active library already contains a voice with the simular filename.
- An overwrite warning is displayed when a voice with simular filename exist.
- The voice is stored into /VL-Voice/[library] and the library contents table is updated.
How to manage voices?
```
To save a voice into another library;
```
- select another library.
- press to save the voice.
```
To save a voice using another name;
```
- change the DISPLAY and/or ELEMENT NAME. Click these fields in Main Control Center.
- press to save the voice.
- To save a voice into a new category;
- change the CATEGORY NAME or select an existing one. Click the category field.
- press to save the voice.
- the voice will appear in the (newly created category of the active library.
```
To save a voice as Favorite;
```
- hoover with the mouse over the voice picture.
- press to copied the voice into the [Favorites] library.
11
```
TIP: If somethings goes wrong then and the MCC does not become active then click on the 3
```
LEDS to send a MIDI PANIC message. This will re-initialize the VL-Wizard and send the midi
commands to the VL-device to kill notes etc..
```
To delete a voice from a Library;
```
- use the file manager to go to the /VL-Voice/[library]and delete the filename.
- go back to the Voice Browser and double click on the library name
- a dialog will show-up, press proceed to rescan the folder of the library.
```
To copy a voice into another library;
```
- load the voice.
- select the destination library.
- press to save the voice.
```
To move a voice into another library;
```
- go to the VL-Voice folder on your disk and open the library folder.
- select the correct voice filename and press Ctrl-X (cut).
- open the folder of the destination folder and press Ctrl-V (copy).
- go back to the Voice Browser and you will see that voice is still visualized.
- double Click the voice you just move to another library.
- a dialog will show-up telling that it can not find anymore the voice in the library.
press the [Remove] button to remove the voice from the inventory.
- now select and double-click the library where you copied the voice into.
- a dialog will show-up, press proceed to rescan the folder of the library.
- the voice you have moved will now appear in the library.
```
To save a voice as a midi file format 1 (Premium Feature);
```
- load the voice.
- hold the key [Shift] while clicking clicking the save button
- a new voice file will be written to the /VL-Voices/[Library]/voicename.mid
12
Section 6
The Voice Browser
13
Remove selected voice
Load FX tables from selected voice
Load all tables from selected voice
Search voice within library
Library cover
Voice Comments
Voice Image
Voice SelectorCategory SelectorLibrary Selector
Voice Category
Voice Display Name
What is the Voice Browser?
The Voice Browser is a tool designed to keep your VL-voices well organized. It provide you a structured
overview of all the Voices and allows you to quickly navigate and find the voice you are looking for. Addi-
tionally it allows you to load effects, select voice templates, import new voices and older Yamaha Librar-
```
ies (VL70m format .ALL .LIB)
```
Each voice is stored individually as a sysex file into a library folder. The main folder /VL-Voice does con-
tain subfolders. Each subfolder represents a library. There are also 4 special subfolders used for [Favor-
ites], Effects, Images , Templates. The purpose and contents of the library folders is explained in this
chapter
How to load a voice?
When opening the Voice Browser you see 3 columns, Library, Category and Voices. Click in the first col-
umn to select a library, the second to select a category and the third to select the voice. There are 4
```
ways to load a voice;
```
```
1) Select a voice with the mouse cursor and [Double-Click /Left-Mouse button]
```
```
2) Select a voice with the mouse cursor, use [Cursor Up/Down] and press [Enter]
```
```
Attention: [Enter] does only work when 3rd column is active (see Red frame)
```
```
3) Select a voice with the mouse cursor, then click
```
```
4) Send a Midi Bank/Program change to the VL-Wizard (see further in this manual)
```
The voice will be loaded into the selected Voice Slot#. The Voice Slot selector is part of the Main Con-
trol Center and is only visible when you configured the VL-Wizard to work with multiple VL-Devices. The
Voice Browser will automatically take focus when double-clicking on the mini-thumbnail of the MCC-
Voice Slot Selector.
About organizing Libraries
To create a new library you simply make a new subfolder in /VL-Voices and give this the name of the
```
library you want to create. You can include a Library cover image into this folder (see examples in the
```
```
other libraries folders). You need to create a file type PNG image, use the same name as the library
```
folder it is located in and use a size of 362 x 604 pixels. After you have done this you re-open the Voice
Browser and your new library will appear. You can make as many libraries as you want, but a better ap-
proach is keep them limited and use different Voice Categories instead.
```
Now you probably want to copy some voices into your new library. There are two approaches for this;
```
```
1) The easiest way, open the Voice Browser, load the voice you want to copy into your new Library, se-
```
lect your new library and press the save button.
```
2) The more complicated approach, manually copy the voice sysex files into your new library by using
```
the Windows file manager or Mac Finder. If you do this then these voices will not automatically appear
14
in the Voice Browser, you will have to force a re-indexing by going to the Voice Browser, Select and dou-
ble click the library name. Re-indexing forces the Voice Browser to re-scan the selected library sub-
folder and analyses the contents of each voice located in that folder. The results of this is stored into
the VLVoiceDB.JSON file of the library.
Why are the VL-Voice sysex files larger than 1808 bytes?
The VL-Wizard stores extra information into a voice sysex file. These are the comments, reference to
the Voice image, author, category etc. This extra voice information is only used by the VL-Wizard and is
ignored when sending to a VL-device. It is written beyond byte position 1808. Many tests have proven
that this approach does not interfere with the midi sysex communication protocols.
The Images folder
This folder contains images that can be assigned to a voice using the little camera icon that ap-
pears when hovering over the Voice image in the MMC screen. You can create your own images and
add them into this folder but when you distribute your voices to other VL-users then must include your
new images and tell them to copy them to the ../VL-Voices/Images folder. Otherwise other VL-users
will not see the images you did assign.
The default VL-WIZARD image will appear when the Voice Browser does not find the images referred by
the loaded voice.
The Effects folder
This folder contains the effect voices. When the Voice Browser loads such voice then it will only take its
```
effects parameters and not the entire voice (element etc). It will not overwrite the element parameters
```
of the loaded voice. Using this approach is useful to quickly load new effects. You could trigger the load-
ing of an effect voice by sending a Bank/Program Change message to the VL-Wizard.
The Voice Browser recognizes an effect voice if it belongs to the category EFX and has an element
name called “Effects”. So, you could actually store effect voices in any kind of library but this is not rec-
```
ommended. You can create your own effect voices by doing following;
```
```
1) Tweak the effect parameters the Effect Editor
```
```
2) Set the Voice Element name towards “Effects”
```
```
3) Set the Voice Category to “EFX”.
```
```
4) Save the voice into the Effects Library.
```
15
```
TIP1: You can also select an effect voice with the dropdown box located in the left
```
lower corner of the effects editor.
```
TIP2: The little FX icon in the Voice Browser offers another way of loading effects.
```
Click this icon allow to load the effects parameters of any selected voice.
The Templates folder
```
Important: Never change the contents of this folder.
```
Each file in this folder is a special template voice. A template voice contains the initial parameters to
start building a voice from scratch. When selecting the Templates library in the Voice Browser the area
```
where you normally see the Library cover image changes into a Template selector (filter). You can select
```
a DRIVER and BODY type. The voice templates corresponding to this selection criteria will show-up.
You can load the template voice and start using it as a base for designing your own new VL-Voices.
You will see that you can not import or delete template voices. These button are disabled when the Tem-
plate Library is activated.
16
Driver Selector
Body Selector
Comments on the
Selected template
Image of the Selected
template
Template voices that fit the se-
lected Driver & Body criteria
Template voices that fit the se-
lected Driver & Body criteria
The [Favorites] folder
The VL-Wizard gives you a quick way to store a voice into the [Favorites] library. To do this you hover
over the voice image in the Main Control Center screen then an little star-button appears. Click this
button to store the loaded voice into the [Favorites] library.
If you want to remove a voice from the [Favorites] library you must manually delete its voice sysex
file from the ../VL-Voices/[Favorites] folder, then select and double click the [Favorites] library
in the Voice Browser to re-index this library.
The Voice Browser import feature
The Voice Browser also offers a powerful import function and allows you to import voices from a .LIB
.ALL or .MID file. Click the import button to select a file you want to import.
During the import process the VL-Wizard will verify if everything is conform to the VL70m format, then it
```
will try to categorize the voice(s) based on the contents of its Display and Element name.
```
Searching a voice using the Voice Browser
You can enter any kind of search text into the search field and press [Enter] key. The informa-
tion contents of all voices in the selected library is checked and voices that correspond to the search
criteria will be visualized.
```
Loading Voices by sending a Bank/Program Change (Premium)
```
When the Voice Browser is open it listens to the Midi-In port for a specific BANK SELECT MSB/LSB. If
both the MSB and LSB are 99 or 63H then it will take the Program Change Number and scan the se-
lected library for a voice that contains the corresponding Program Change Number in its comments. Ex-
```
ample: When receiving the following: B0H 00H 63H, B0H 20H 63H, C0H 04H the Voice that
```
contains [PC004] in the comments will be loaded and transmitted to the VL70m.
This feature only works when the Voice Browser is open and the selected library contains a voice with
the corresponding tag. If no voice found then the Program Change is passed through to the VL70m.
17
```
RESTRICTION: Patchman voices can not be imported because these voices are Intellectu-
```
ally Protected. This has been done on request of Patchman Music.
Section 7
The Editors
Introduction to the 4 editors
The common editor is used to edit the voice parameters that are also available via the VL70m front
panel menu. It provides you access to the assignable controllers, note scaling etc. You also have a sec-
tion for setting the VL70m system parameters, these system parameters are not part of the VL-Voice.
These parameters are made available to make it easier for you to take full control over your VL70m with-
out the need to touch the front-panel menu.
The element editor is the real thing and provide in-depth access to all the parameters that create the vir-
```
tual instrument (VL voice). From within the element editor you can call-up the Key-Scale/Break point edi-
```
tor.
The tuning editor is designed to help you to retuning each single note of a voice, you have a manually
and automatic tuning option.
The effects editor is designed to set the effect parameters of a voice. It is important to know that this
editor is only working for VL70m units and not for PLGVL cards. It is also important to know that the ef-
fect parameters are part of the common parameters.
A bit more information about a VL-Voice structure
One single VL70m voice consists of 15 sysex tables representing a total of 1808 byte.
The largest and most important sysex table is called the VL-element. This table contains all parameters
that influence the behavior of the VL-model. The element parameters are divided in 3 large groups, the
```
Driver-Body, the Modifiers, the Tuning buffer (Pipe/String length). The Driver-Body and Modifier parame-
```
ters can be edited with the ELEMENT EDITOR, while the Pipe/String Length parameters can be set with
the TUNING EDITOR.
The VL70m has 6 CUSTOM memory slots. The CUSTOM memory slots can store the element parame-
ters but they are not able to include all the common parameters. This is why recalling a voice via the
CUSTOM bank will never reproduce the exact same sound. Common parameters of a voice must be
stored into one of the 64 INTERNAL slots. These INTERNAL slots have less memory space and can
only hold the common parameters. This is the reason why each INTERNAL slot is pointing to a slot in
the CUSTOM or PRESET banks. You can see the bank pointer of an INTERNAL slot by when pressing
the PART [-]and[+] simultaneously on the front-panel the VL70m. You can even change the bank
pointer via the front panel menu. The VL70m display you show two voice names, the one on the right
side is the element voice. Use the SELECT [<]and[>] and VALUE [-]and[+] buttons to change
the bank pointer. The VL-Wizards offers a tool called the CST/INT tool. This tool also allows you to
change the bank-pointer of the loaded voice. See section “How to use the CST-INT tool?”
18
```
The VL-Wizard offers a tool (snippets) that allows you to record, store and recall the parameter changes
```
you did on the common, effect and element editors. This is a very handy tool for recalling frequently
used parameter settings/combinations. Example: You could make a set of different EQ settings and
store each of them on your disk so you can recall them anytime. See section “How to use the snippet
tool?”
In some cases you want to recall the initial value of a parameter you just changed. The VL-Wizard offers
a kind of undo for most parameters and will attempt to restore the initial value of a parameter when you
double-clicking on it.
The VL-Wizard also allows you to change the look and feel of the common and the element editor. You
can do this by opening the settings selecting the a different colorscheme. You could also decide to de-
fine one yourself by selecting “User Defined”. When you do this a color selector will appear.
19
Section 8
Element Editor
20
The SELECTOR: clicking any
label will put the section in focusThe ALGORITHM selector
Keyscale-
Breakpoint button
Section title
The element editor is the place where you set the parameters that influence the way the VL-voice
sounds and responds to the various controller parameters set in the common editor.
At the left side of the element editor you see the SELECTOR. Each label refers to a parameter section.
Clicking on a label will scroll the element editor immediately to that parameter section, eg. Click on Fre-
quency Equalizer and see how this section of parameters gets focus.
```
A very important parameter is the VL-Algorithm which sets the routing of the VL-signal and (de)acti-
```
vates certain parameter groups. The parameter sections that are switched on/off by the VL-Algorithm
are NOICE, THROAT, DYNAMIC FILTER-MODULATOR, RESONATOR. Below the VL-Algorithm parame-
ter you find a graphical representation on how the signal is routed and which sections are switched on
or off.
The Keyscale / Breakpoint Editor
You will also see that many parameters have a little side button. This button does open the
KeyScale/BreakPoint editor. The KS/BP editor allows you to define how the value of the associated pa-
rameter evolves in relation to the key number that is being played. This is a very interesting feature be-
cause it allows you to can change the behavior of the VL-Model based on the key you are playing.
The amount of BreakPoints do vary for each parameter. Typically parameters that influence the voice
tuning, timbre and stability have more BreakPoints available.
To change the position of a BreakPoint you select the little dot and drag it to the place you want. You
```
will see that moving a dot (BreakPoint) does influence the level and key.
```
Some parameters offer MIN/MAX values, for these you will see a slider at the left and right side and two
```
curves, one of these curves is greyed out (not in focus). To edit the MIN scale you need to click the MIN
```
slider first, to edit the MAX scale you click the MAX slider. The curve associated with the selected slider
will become editable.
You can also set the Breakpoints and their values using the little table under the graph. The first row are
the notes and below the levels. Click and drag the mouse up or down to change the value, or click
once and use cursor up/down.
You can move parts of the entire curve using when settings the mode switch to SHIFT. The mode FREE
does move limit the movement of a breakpoint between it
2122
LEVEL SLIDERLEVEL SLIDERBREAKPOINTS
LEVELS
BREAKPOINT NOTE
EDIT MODE
REGISTER KEY OPEN POSITION
Section 9
Tuning Editor
23
```
Switch Tuning Mode: Pipe-String Length or Derivative (50 Cents)
```
Set buffer length:
small steps
Set buffer length: large steps
Note selectorUndo all tuning
Launch the automatic tuning mode of the VL70m and
tune the notes between the start - stop range
Adjust the automatic tuning parameters
These parameters only become visible with
Child Reed Resonance Control = Relative.
See the Element Editor.
Send Note ON or OFF.
Manual Tuning
Automatic Tuning of singe note
Single Note to be tuned.
The Tuning editor is allows you to tune each individual note of the loaded voice. You can either tune
each note manually or automatically.
Using the Automatic Tuning
This method only works for a VL70m module. The VL70m midi-in and -out port are required to support
the two-way communication automatic tuning protocol. The VL-Wizard does send instructions to the
VL70m to launch the automatic tuning between a start/stop note, this together with the tuning setting
parameters. Once the VL70m receives the automatic tuning instruction it will switch into a special mode
and try to tune each note within the give start/stop range. For each note being automatically tuned a
confirmation message is sent back. The tuning editor will display if a note was successfully tuned
```
(green key) or failed (red red). Hereby a step-by-step overview on how to use the automatic tuning
```
```
method;
```
```
1) Switch off the WX5 to ensure no pitch /note changes are send.
```
```
2) Press the Midi Panic to reset all controllers.
```
```
3) Set the note range and press the green tuning button.
```
```
4) The tuning button will turn red (busy) and the parameters will be dimmed.
```
```
5) The VL70m display will indicate that the Tuning process is busy.
```
```
6) The notes between F3 and E4 will turn green (or red).
```
```
7) The tuning button turns back to green when tuning is completed.
```
```
8) If the tuning failed then you can press the restore button to reload the initial tuning tables of the
```
loaded voice.
The automatic tuning settings can be adjusted if the tuning does not work well. Very little information is
available on the settings that influence the tuning process.
If the buffer length value of a note is too far out of the range then the automatic tuning will fail for that
note. You can manually set the buffer length value so if gets back into the range where the automatic
tuning works.
Using the One Note Automatic Tuning
This method is the same as the Automatic Range Tuning. The difference is that it only autotunes the se-
lected note instead of the selected range.
24
The Manual Tuning
You might prefer to use this method above the automatic tuning because it allows you to stay under
full control during the tuning process. This method requires to include an accurate external tuner or soft-
ware tuner that can analyze the audio output of the VL70m and visualize if the note is within the accept-
able tuning range. In this example we use the Motu 828x CueMix Tuner. Hereby the steps to take for
```
manual tuning;
```
```
1) Ensure the external tuner is listening to the VL70m audio output.
```
```
2) Switch off the WX5 to ensure no pitch /note changes are send.
```
```
3) Press the Midi Panic to reset all controllers.
```
```
4) Select the note to be tuned via the virtual keyboard or listbox.
```
```
5) Press the Note ON/OFF button and adjust the breath level towards 80%
```
```
6) The selected note will sound, adjust now the VL70M volume.
```
```
7) The external tuner does show that the F3 note is out of tune.
```
```
8) Adjust the buffer length till the F3 note is in-tune. Use Click and drag up/down
```
to adjust the pitch. Up = higher pitch, down = lower pitch.
```
9) Select another note to be tuned and repeat the above steps.
```
25
More about Tuning
Set the voice effects to DRY: It is recommended to set all voice effects to dry during the tuning with
an external tuner. This way the audio output of the VL70m is clear and it will be easier for the external
tuner to catch the frequency of the note.
```
Set the KS/BP values of Pitch Compensation to zero (see element editor): It is recommended to
```
set the KeyScale/BreakPoint values of the voice Pitch Compensation to zero before starting the actual
tuning process. Once the tuning process is successful you can readjust the Pitch Compensation val-
ues.
Following parameters groups have considerable impact tuning the voice:
- Embouchure / Friction
- Beat
- Pitch Compensation
- Pressure
- Parent Reed
- Child Reed
- Throat
- Pipe / String
```
The Tuning Editor Derivative Mode (50cents): Using this mode allows you to adjust each note with
```
50cents.
26
Section 10
Common Editor
The common editor does allow you to edit the parameters that are accessible via the front-panel menu
of the VL70m. You will quickly recognize these parameters. You can easily see the interaction between
the common editor and the VL70m. Simply navigate with via the VL70m front-panel menu to any pa-
rameters and then change its value via the common editor. You will see that the VL70m display follows
the value of the parameter changes you do in the common editor.
The common editor has also a section called SYSTEM. This section allows you to edit the system pa-
rameters of the VL70m. You load the current system settings of the VL70m into the VL-Wizard by click-
ing the little Midi-WX icon.
27
Section 11
Effect Editor
The Effects editor does only work for VL70m units. There are 4 effect blocks. The routing switch allows
you change the path of the effect signal. Each effect block has a listbox allowing you to load a specific
settings. The effect parameters are set to their default values when changing the type of effect. The pa-
rameters of the Distortion and Variation block are enabled of disabled based on the status of the ON/
OFF switches. In the lower left area you find a listbox allowing you to quickly load another effect tem-
plate. These effect templates part of the Effects library and can be found in the /VL-Voices/Effects sub-
folder. The effect parameters are saved together with each voice.
28
DISTORTION
effect block
VARIATION
effect block
REVERB
effect block
CHORUS
effect blockDISTORTION
effect block
ON.OFF switch
VARIATION
effect block
ON.OFF switch
EFFECTS
Routing switch
EFFECT TEMPLATE
selector
EFFECT TYPE LISTBOX
Section 12
Settings
29
AREA AAREA B
AREA C
AREA C
AREA D
INTERVAL BETWEEN DUMPOUTS
This setting defines the delay interval between each individual voice sysex table being
send to the VL70m. During the initial setup you better keep this to a value of 50ms. You
can bring this value down to 0ms you see that you have a reliable communication with the
VL70m. If you see a sysex error on the display of your VL70m then you will have to keep
this value to 50ms or higher.
INTERVAL BETWEEN BYTES
Similar to the above, this settings allows you to define the delay interval between bytes.
Again, take a value of 50ms to start with and bring this down when you have no errors on
the VL70m display.
FORCE VOICE LEVEL
This setting allows you to set a default volume level for each voice you want to load. Put-
ting this value to 0 does disable this feature, meaning that the voice level will be loaded as
original stored in the voice parameters.
DEVICE CHECK
This feature is explained in the “Getting Started” chapter. See Step 12.
VOICE MODE
This setting is only relevant when using multiple VL-Devices. It forces the VL-Device Note
Mode into Poly or Mono. Keep this to Mono when using a single VL70m. .unit
You will see that this settings can be MONO, POLY, UNISON, EXPERT.
Using MONO mode tells the VL-Wizard to force each VL-device into Mono and transmit to
each of them the same Voice.
Using POLY tells the VL-Wizard to switch each connected VL-Device into Poly and trans-
mit to each VL-device the same voice. Each VL-Device will also get a note number as-
signed. So, pressing one key = VL-device number 1 will sound, pressing a second key and
VL-Device number 2 will also sound etc...
30
```
TIP: The VL70m can load a complete voice in about 2 seconds when the Interval times are set to a
```
value of 0ms. A lot depends on the ability of the Midi-interface to transfer large sysex blocks.
```
Using UNISON mode (or multi) allows you to send to each single VL-Device a voice. This
```
is done using the voice slot switching tool that becomes available when using multiple VL-
Devices in UNISON or EXPERT mode.
The EXPERT mode is the similar as UNISON mode, with that difference that the VL-Wizard
allows you to overrule the MONO or POLY parameter.
The UNISON/EXPERT mode allows you to layer voices and make interesting combina-
tions. Example combining a Upright bass with a AltoSax. load a Tenor of the lower and
Alto Sax for the higher ranges. etc.
TOTAL UNITS
This setting tells the VL-Wizard if it has to handle multiple VL-Devices. Up to 8 VL-Devices
are supported. The total rows of the Device Assign settings does reflect the Total Units
number.
PLGVL mode
When you selected a PLGVL type in the Device Assign section then the VL-Wizard will use
this parameter to decide what sysex tables to send to the VL-Device. There are 4 mode A,
B, C, D. In most case use mode A for a PLGVL card. But Mode D also works for MU units.
```
A: table 15
```
```
B: table 1 2 3 4 5 15
```
```
C: table 1 2 3 4 5 14 15
```
```
D: table 1 2 3 6 7 8 9 10 11 12 13 14 15
```
nr address size
01 -- 08 0p 00 Table 7a 29 41 Current Voice / Common Part Parameter
02 -- 08 0p 30 Table 7b 3F 63 "
03 -- 08 0p 70 Table 7c 04 04 "
04 -- 09 0p 00 Table 7d 17 23 "
05 -- 09 00 17 Table 7e 06 06 "
06 -- 02 01 00 Table 8a 0E 14 Current Voice / Common Effect Parameter
07 -- 02 01 10 Table 8b 06 06 "
08 -- 02 01 20 Table 8c 0F 15 "
09 -- 02 01 30 Table 8d 06 06 "
10 -- 02 01 40 Table 8e 21 33 "
11 -- 02 01 70 Table 8f 06 06 "
12 -- 03 00 00 Table 8g 12 18 "
13 -- 03 00 20 Table 8h 06 06 "
14 -- 10 00 00 Table 6 0F 15 Current Voice / Common Misc Parameter
15 -- 20 00 00 Table 8i 56B 1387 Current Voice / Element Parameter
31
FORCE EFFECT DRY
This setting does tells the VL-Wizard to overrule the effects that are included in the VL-
voice and force them to DRY during the loading.
FORCE NATIVE HEADERS
This setting tells the VL-Wizard send each sysex table using the VL native header and not
the VL-XG header. This feature is included to ensure maximum compatibility. Best is to
keep this setting ON= force native header.
COLOR THEMES
See Chapter 6, general information on the different editors.
DEVICE ASSIGN TABLE
The setting “Total VL-Units” does set the number of rows available. Each row represents a
connected VL-Device.
DEVICE ID# : It is recommended to provide each VL-device a specific device ID. You have
to do this on the Yamaha synth itself. The advantage of give each device a separate ID is
that you are sure that the sysex data will only be received by that device. This is very use-
ful with working with complex setups in Unison/layered mode.
PART# and SERIAL# are settings that are available to allow for maximum compatibility,
normally you will not need to use these.
NOTE# is used in Poly mode. This settings is automatically set by the VL-Wizard when set-
ting the “Voice Mode” to Poly. You can overrule this setting anytime.
TYPE us used to tell the VL-Wizard that this slot is a VL70m or PLGVL.
MIDI OUT port name and CHANNEL is set for each VL-Device slot.
Use the little Refresh button to reload the midi-port names. This is useful when you
connected a midi-interface after you did launch the VL-Wizard.
The field LIBRARY does contain the path towards the VL-Voice. This is explained in the
“Getting Started” chapter - step 3.
The field SYSEX LIST does contain the path where you store all your midi and sysex files
that you use to configure your synth. See chapter “Widgets” on how to use this feature.
The field INIT FILE does contain the path and filename of the midi or sysex that is send to
your Yamaha synth during start-up of the VL-Wizard. This can be handy to force your
synth into VL-mode.
32
Section 13
Widgets
The USER SYSEX LIST WIDGET
```
This tool is used to send special instructions to the Yamaha synth containing the PLGVL card(s). You
```
need to create a folder with text files containing sysex instructions. The folder path has to be specified in
the SYSEX LIST field of the VL-Wizard settings.
The text file can contain sysex and controller instructions. Hereby an example of the contents for switch-
ing a Yamaha S90 into PLGVL mode.
F0 43 10 6B 0A 00 01 00 F7
F0 43 00 6B 00 30 00 10 00 00 01 02 00 40 00 00 00 00 00 00 00 00 00 00 00 00 01 02 00 40 00 00 00 00
00 00 00 00 00 00 00 00 01 02 00 40 00 00 00 00 00 00 00 00 00 00 00 77 F7
BF 00 3F BF 20 18 CF 00 BF 00 21 BF 20 01 CF 04
F0 43 10 6B 4C 00 0B 10 F7
F0 43 10 6B 4C 00 0C 10 F7
F0 43 10 6B 4C 70 09 2E F7
F0 43 10 6B 4C 70 00 56 F7
F0 43 10 6B 4C 70 01 4C F7
F0 43 10 6B 4C 70 02 2E F7
F0 43 10 6B 4C 70 03 57 F7
F0 43 10 6B 4C 70 04 69 F7
F0 43 10 6B 4C 70 05 7A F7
F0 43 10 6B 4C 70 06 20 F7
F0 43 10 6B 4C 70 07 2E F7
F0 43 10 6B 4C 70 08 4D F7
F0 43 10 6B 4C 70 09 2E F7
F0 43 10 6B 4C 00 03 00 F7
```
You can also include the word “CHECKSUM” before the End Of Sysex Byte (F7h). This will instruct the
```
VL-Wizard to calculate the Yamaha checksum byte.
The Store to CST/INT WIDGET
This tool is used to store a voice from the VL-Wizard library into the VL70m CST/INT memory bank.
To correctly reproduce the sound of a voice you always need to store a voice into both the CST and INT
slot. The tool is doing this for you.
```
1) Load the voice into the VL-Wizard using the Voice Browser.
```
```
2) Open the “STORE TO CST & INT” tool.
```
```
3) Select the desired CST slot number and press the “STORE TO CUSTOM button.
```
```
4) You will see that the tool is setting the PGM number of the INTERNAL bank to the same as the
```
selected CST number.
```
5) Press now the “STORE TO INTERNAL” button.
```
33
To recall the voice you need to select the INT number on the front panel of the VL70m and not the CST num-
ber. The reason for this is because the INTERNAL memory slots contains additional data that influences the
sound of a voice. Selecting the CST will not correctly reproduce the sound.
The MIDI LEARN WIDGET
This tool is used to map external midi controllers to almost any parameter in the VL-Wizard editors. First make
sure that you correctly receive midi CC data from your external midi controller. This is done by going to the set-
```
tings and switching ON the midi-in port(s) that are connected with your hardware controller.
```
```
1) Open the Common, Effect, or Element editor.
```
```
2) Open the “MIDI LEARN” tool.
```
```
3) Click the Record button (ON/OFF) - The button will turn red
```
```
4) Send a Midi controller data by moving a midi-controller knob, slider
```
-> The “MIDI LEARN” tool will show incoming CC numbers
```
5) Change a parameter within the Common, Effect or Element editor
```
-> The “MIDI LEARN” tool will show the mapped VL parameter/
```
6) Change the value of the external midi-controller and see how the
```
associated VL parameter within the VL-Wizard reacts in real-time.
The MIDI PHRASE LOOPER WIDGET
This tool is used to play and record single track midi files. This allows you to play a continuous midi loop while
editing parameters of the loaded voice.
```
1) Open the “MIDI PHRASE” tool
```
```
2) Load a single track midi file and press [START] to playback
```
```
3) Or press the [REC] to record a phrase you are playing
```
```
4) Press again the [REC] button to stop recording
```
```
5) Press [START] to play.
```
```
6) Press [SAVE] to store the recordings to a midi file (format 1).
```
The SNIPPET WIDGET
This tool allows you to store and reload a set of parameters. Additionally to this you can record a given set of
parameters changes, store and reload them. With other words, this tool allows you to store the parameter
changes that you want to re-apply to other voices.
```
1) Load a voice containing settings you want to re-use for other voices.
```
```
2) Open the “SNIPPET” tool.
```
```
3) Check the checkboxes of the parameter sections that you want to store.
```
```
4) Press [SAVE] button on the snippet tool and store the file into a folder of choice.
```
34
```
5) Load another voice.
```
```
6) Press [LOAD] button on the snippet tool and load the file containing the parameters that you want
```
to re-apply.
```
7) See now how the parameter values of the loaded voice change to the once that are stored into
```
the snippet file you did load.
You can store any kind of parameter into a snippet file. Simply press the [REC] button on the snippet
tool, go to the Effect, Common or Element editor, start changing the parameters that you want to re-
apply to other voices. The “SNIPPET” tool will record your individual parameter changes. Press [REC]
again to stop the recording and repeat the actions as from step 4.
35
```
ATTENTION: some VL-Parameters do respond well to real-time changes while playing the VL70,
```
while others do not. Some parameters can not be mapped to a CC as because they would cause is-
```
sues with midi-data transfer. (eg. Tuning, Keyscaling)
```
Section 14
VL70 Architecture
36