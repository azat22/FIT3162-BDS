# Welcome to Better Display Server! 

This is the first step to getting started in your journey to using Hipe. 

The first step is to get Hipe installed on your system. We are assuming that you are using the *Ubuntu* distribution of Linux (version 20.04). 

**Feel free to skip this guide if you already have Hipe installed on your system.**

This guide is only for running Hiped within an existing desktop environment. If your intention is to deploy Hipe in an embedded environment, this might not apply to you.

The source code for Hipe is located [here](https://gitlab.com/danielkos/hipe). If you know your way around Git, you can clone it with this command: 

``` 
git clone https://gitlab.com/danielkos/hipe.git 
```

You can also go to the link above, and click the download icon to download the source code as as .zip file (or whatever other compressed format you prefer).

Go ahead and unzip the file somewhere convenient (such as in the Desktop), and you should have a folder called 'hipe-master'. 

Within it, you'll find the *server* and *api* folders. First, we'll install the hipe server. 

Hipe utilises QtWebKit, and we'll install it with this command:

```
sudo apt install libqt5webkit5-dev
```

When prompted to continue, type Y and enter. Let the package installer do its thing, and QtWebKit will be installed on your system. 

Next, run this command to ensure Qt5 is installed on your system:

```
sudo apt install qt5-default
```

Ensure you have the correct Qt version with this command:

```
qmake --version
```

You should see something like:

*QMake version 3.1
Using Qt version 5.12.8 in /usr/lib/x86_64-linux-gnu*


Congrats, you have the dependencies sorted out. Now, we can install hiped. 

Navigate to the *hipe/server/* directory, and run the following command:

```
qmake -makefile ./src/hiped.pro
```

Next, run this simple command:

```
make
```
This should build all the hiped files and populate them in the *server* directory. 

Within the same directory, you can run the executable file with this command:

```
./hiped
```
You should get an output in the terminal like:

*hiped: Listening on /run/user/1000/hipe.socket*

Once you see this, hiped has been successfully installed on your system. 
 
Currently, you can only run hiped from within the *server* subdirectory, which can be inconvenient. We'll turn 'hiped' into an alias so we can start it from anywhere in the OS. 

There are multiple ways of doing this, but we will present one possible method:

- Run ``` mkdir ~/bin ``` to create a ``` bin ``` directory in the home directory.
- Move the *hiped* executable file over to this folder with this command: ``` mv hiped ~/bin ```
- Run this command: ``` chmod +x ~/bin/hiped ```
- Open the ~/.bashsrc file with this command to modify it: ``` sudo nano ~/.bashsrc ```
- Add this line: ``` export PATH=$PATH:~/bin ```
- Save the file and exit (Ctrl + O, enter)
- Run this to refresh the commands list: ``` source ~/.bashsrc ```  
- And then, run ```hiped```

And voila! You are now able to get a hipe session running from anywhere in the OS. 

Now, we have to install the Hipe API library. Navigate to *hipe-master/api/*.

Run ```make``` to build the API library, libhipe.

Next, run ``` sudo make install ``` , and then ``` sudo make testing ``` to install libhipe as well as the test files. 

Once done, that's it. Hipe is now installed on your system and ready to run!

Open two terminals:
- On one terminal, run ``` hiped ``` to start the hiped server.
- On the other terminal, navigate to *hipe-master/api/test* and run ``` ./hipe-calc ```

You should now see a beautiful sample Hipe-based program. Similarly, you can run the other test programs to get a feel for some of the Hipe functionality. 

That's it for the installation guide, you can now move on to making your own Hipe application, [To-doist!](making_todoist.md)

