#pragma once

class IDevice;
class IRecorder
{
public:
	virtual ~IRecorder() { }

	virtual bool create() = 0;

	virtual void start()
	{ recording = true; }

	virtual void stop()
	{ recording = false; }

	virtual void write(void* frame, int stride) = 0;
	
	bool isRecording()
	{ return recording; }

protected:
	IRecorder() : recording(false) { }

private:
	bool recording;
};