# Configure local API key using https://www.datacamp.com/tutorial/converting-speech-to-text-with-the-openAI-whisper-API

from openai import OpenAI
client = OpenAI()

if __name__ == "__main__":
  audio_file= open("samples/sampleButterflies.wav", "rb")
  transcript = client.audio.transcriptions.create(
    model="whisper-1",
    file=audio_file
  )

  print(transcript)
