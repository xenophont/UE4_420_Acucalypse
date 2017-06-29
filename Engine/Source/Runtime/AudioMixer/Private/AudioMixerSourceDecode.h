// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Classes/Sound/SoundWaveProcedural.h"

namespace Audio
{
	class FMixerBuffer;

	// Data needed for a procedural audio task
	struct FProceduralAudioTaskData
	{
		// The procedural sound wave ptr to use to generate audio with
		USoundWaveProcedural* ProceduralSoundWave;

		// The audio buffer to fill from the results of the generation
		uint8* AudioData;

		// The size of the audio buffer
		int32 MaxAudioDataSamples;
	};

	// Data needed for a decode audio task
	struct FDecodeAudioTaskData
	{
		// The mixer buffer buffer object which holds state to use for the decode operation
		FMixerBuffer* MixerBuffer;

		// A pointer to a buffer of audio which will be written to
		uint8* AudioData;

		// The number of frames to decode
		int32 NumFramesToDecode;

		// Whether or not this sound is intending to be looped
		bool bLoopingMode;

		// Whether or not to skip the first buffer
		bool bSkipFirstBuffer;
	};

	// Data needed for a header parse audio task
	struct FHeaderParseAudioTaskData
	{
		// The mixer buffer object which results will be written to
		FMixerBuffer* MixerBuffer;

		// The sound wave object which contains the encoded file
		USoundWave* SoundWave;
	};

	// Results from procedural audio task
	struct FProceduralAudioTaskResults
	{
		int32 NumBytesWritten;

		FProceduralAudioTaskResults()
			: NumBytesWritten(0)
		{}
	};

	// Results from decode audio task
	struct FDecodeAudioTaskResults
	{
		bool bLooped;

		FDecodeAudioTaskResults()
			: bLooped(false)
		{}
	};

	// The types of audio tasks
	enum class EAudioTaskType
	{
		// The job is a procedural sound wave job to generate more audio
		Procedural,

		// The job is a header decode job
		Header,

		// The job is a decode job
		Decode,

		// The job is invalid (or unknown)
		Invalid,
	};

	// Handle to an in-flight decode job. Can be queried and used on any thread.
	class IAudioTask
	{
	public:
		virtual ~IAudioTask() {}

		// Queries if the decode job has finished.
		virtual bool IsDone() const = 0;

		// Returns the job type of the handle.
		virtual EAudioTaskType GetType() const = 0;

		// Ensures the completion of the decode operation.
		virtual void EnsureCompletion() = 0;

		// Returns the result of a procedural sound generate job
		virtual void GetResult(FProceduralAudioTaskResults& OutResult) {};

		// Returns the result of a decode job
		virtual void GetResult(FDecodeAudioTaskResults& OutResult) {};
	};

	// Creates a task to decode a decoded file header
	IAudioTask* CreateAudioTask(const FHeaderParseAudioTaskData& InJobData);

	// Creates a task for a procedural sound wave generation
	IAudioTask* CreateAudioTask(const FProceduralAudioTaskData& InJobData);

	// Creates a task to decode a chunk of audio
	IAudioTask* CreateAudioTask(const FDecodeAudioTaskData& InJobData);

}