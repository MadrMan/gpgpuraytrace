#pragma once

class IDevice;

//! Interface for all recorder classes
class IRecorder
{
public:
	//! Destructor
	virtual ~IRecorder() { }

	//! Create the recorder
	virtual bool create() = 0;

	//! Start recording
	virtual void start()
	{ recording = true; }

	//! Stop recording
	//! \note This NEEDS to be called to properly finish recording
	virtual void stop()
	{ recording = false; }

	//! Write a frame 
	virtual void write(void* frame, int stride) = 0;
	
	bool isRecording()
	{ return recording; }

protected:
	IRecorder() : recording(false) { }

private:
	bool recording;
};