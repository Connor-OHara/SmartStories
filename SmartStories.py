# Configure local API key using https://www.datacamp.com/tutorial/converting-speech-to-text-with-the-openAI-whisper-API

from openai import OpenAI
import cv2, asyncio

client = OpenAI()


async def main():


    #async call to be made during cv processing of images
    async def text_speech():
        return













if __name__ == "__main__":


    #manage and clear some directories





    # process the audio file stored as below
    audio_file = open("samples/sampleButterflies.wav", "rb")
    transcript = client.audio.transcriptions.create(
        model="whisper-1",
        file=audio_file
    )

    # audio file is processed
    print(transcript)
    extractedTranscript = transcript.text
    print("Whisper Response: ")
    print(transcript.text)

    print("Calling ChatGPT with prompt: " + transcript.text)

    #Now we go and call text ChatGPT
    response = client.chat.completions.create(
      model="gpt-3.5-turbo",
      messages=[
        {"role": "system", "content": "Generate discrete paragraphs from the prompt. "
                                      "They are being used in DAL-E to create images to along with is."},
        {"role": "assistant", "content": transcript.text},
      ]
    )

    chat_response = response.choices[0].message.content
    print("ChatGPT Response:")
    print(chat_response)


    #image generation


    response = client.images.generate(
        model="dall-e-3",
        prompt="a white siamese cat",
        size="1024x1024",
        quality="standard",
        n=1,
    )

    #process images
    image_url = response.data[0].url


    #text to speech








