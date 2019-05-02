#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include "Core/Interfaces/Object.h"

class AudioClip : public Object
{
protected:
	std::string _name;
	unsigned char* _data;
	std::size_t _size;
	std::size_t _sampleRate;

	unsigned int _bufferID;

public:
	AudioClip ();
	~AudioClip ();

	void SetName (const std::string& name);
	void SetData (unsigned char* data, std::size_t size, std::size_t sampleRate);
	void SetBufferID (unsigned int bufferID);
	
	std::string GetName () const;
	unsigned char* GetData ();
	std::size_t GetSize () const;
	std::size_t GetSampleRate () const;
	float GetDuration () const;
	unsigned int GetBufferID () const;
};

#endif