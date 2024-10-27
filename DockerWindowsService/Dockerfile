# Use a compatible Windows image
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# Set the working directory
WORKDIR /app

# Copy your executable into the container
COPY C:/Users/Magshimim/Documents/Magshimim/or-akiva-504-parentalcontrolp2p/ParentalControlFINAL_PROJ/x64/Debug/ParentalControlFINAL_PROJ.exe . 

# Command to register and start your service
RUN sc.exe create MyService binPath= "C:\\app\\ParentalControlFINAL_PROJ.exe" start= auto

# Start the service when the container runs
CMD ["sc.exe", "start", "MyService"]
