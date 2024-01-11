# Flight Tracker
The Flight Tracker project is an application designed to provide real-time information about flights within a specified geographical boundary. This project was inspired by a fascination with aviation and the desire to explore the capabilities of integrating external APIs into Arduino and ESP32/8266 projects.

## Key Features
__1. Geographical Boundary Configuration:__ 
* The application defines a boundary box based on coordinates, allowing users to monitor flights within a specific area. In this case, Airbus Headquarters, Blagnac.

__2. API Integration:__
* Utilises the Flight Radar API (flight-radar1.p.rapidapi.com) to retrieve real-time flight data within the specified boundary.

__3. Graphics Display:__
* Using Figma to create a clean graphical display with appropriate resolution (128x64).
* Displays aircraft graphics based on the received data, providing a dynamic and engaging user experience.
* Novel graphics for planes of interest, such as the A320neo, A350 and A380.

__4. User Interaction:__
* A momentary push-button triggers the application to check for flights within the defined geographical area.
* LED indicators offer visual feedback during the data retrieval process.

__5. Wi-Fi Integration:__
*  Features of the WiFiManager library for easy configuration, allowing users to set up and manage Wi-Fi connections hassle-free.

__6. 3D Design:__
* Advanced resin 3D printing technology to manufacture the case, known for its ability to produce high-quality finishes with intricate details.
* Using Fusion 360 to create the three-dimensional design for the casing, taking into account both dimensions and aesthetics.

## Project Motivation
The primary motivation behind this project was to explore the capabilities of incorporating external APIs into Wi-Fi-based Arduino applications. Already familiar with the Espressif family of Wi-Fi technology, the idea of utilising this connectivity in order to create a flight tracker stemmed from a curiosity about real-time data retrieval and visualisation in the context of aviation.

## Future Improvements
### Interactive Touchscreen Upgrade
To enhance the display functionality by introducing interactive features through touchscreen capabilities. The plan involves implementing a capacitive screen, renowned for its superior responsiveness in comparison to traditional TFT screens. This upgrade aims to offer users a more intuitive and user-friendly experience, enabling direct interaction with the flight tracking application through touch gestures.

### Compact Design Through PCB Development
Considering physical improvements, the focus will be on PCB development to optimise the internal layout of components. This approach ensures a more compact and pocket-sized device. Streamlining the arrangement of elements on the PCB contributes to a sleeker design, enhancing the overall aesthetics, anthropometrics and portability of the flight tracker.

### Cellular Capacity
Continuing in a portability-focused direction, considerations will be made to either replace or supplement the existing Wi-Fi functionality with the addition of a SIM card. This strategic enhancement reduces reliance on finding a stable Wi-Fi network to activate the device, making it more versatile and capable of functioning in diverse environments.

## Graphic Design in Figma
To create the graphics for the OLED display, I opted for Figma, a versatile design tool renowned for its intuitive processes, particularly in UI/UX design. The initial step involved establishing a dedicated canvas with precise dimensions, configuring it to a 128x64 pixel resolution to seamlessly align with the specifications of the compact OLED display. Given the limited screen area, I found a plug-in that allowed for the creation of larger, bolder icons, which were imported to enhance visibility. All graphics incorporate both departing and arriving plane symbols, aligning with the source and destination information extracted by the algorithm. Notably, screens featuring planes of interest showcase heart (favourite) symbols, while others display a distinctive rudder-style graphic. Additionally, a designated white band in the design accommodates the flight number (which can be printed in real-time), optimizing the visual representation on the OLED screen and essentially creating a custom graphic each time.

![image](https://github.com/kxvassiliou0/flighttracker/assets/34982747/ddeabc5b-5e9e-4520-b363-40cb252d6b24)
_The above shows my Figma workspace for this project, at the end of my design process._

### Final Images
To enhance the overall look, I incorporated a laser-cut acrylic tinted cover, providing a sophisticated appeal while showcasing the internal components.

![image1](https://github.com/kxvassiliou0/flighttracker/assets/34982747/f969fac8-3aad-4208-84fd-03a7e05d662e)
![image0](https://github.com/kxvassiliou0/flighttracker/assets/34982747/eccfee3d-215e-4e97-818b-1e87e5c7fbe4)
![image3](https://github.com/kxvassiliou0/flighttracker/assets/34982747/f52692e2-80cd-4f52-b5af-76820ec2aa4a)

