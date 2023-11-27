# Configure local API key using https://www.datacamp.com/tutorial/converting-speech-to-text-with-the-openAI-whisper-API

import asyncio
from openai import OpenAI
import openai
import os
import requests
import shutil
import cv2
import numpy as np
import aiohttp
from pathlib import Path
from gtts import gTTS

client = OpenAI()


def download_image(image_url, folder_path, image_name):
    # Create the folder if it doesn't exist
    os.makedirs(folder_path, exist_ok=True)

    # Download the image
    response = requests.get(image_url)
    image_path = os.path.join(folder_path, image_name)
    with open(image_path, 'wb') as image_file:
        image_file.write(response.content)

    return image_path


def text_to_speech(text, output_path):
    tts = gTTS(text=text, lang='en')
    tts.save(output_path)


def calculate_average_color(image_path):
    # Read the image using OpenCV
    image = cv2.imread(image_path)

    # Convert the image from BGR to RGB
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # Split the image into 8x8 grid
    height, width, _ = image.shape
    octant_size = height // 2, width // 2

    # Calculate average color for each octant
    average_colors = []
    for i in range(2):
        for j in range(2):
            octant = image[i * octant_size[0]: (i + 1) * octant_size[0], j * octant_size[1]: (j + 1) * octant_size[1],
                     :]
            average_color = np.mean(octant, axis=(0, 1))
            average_colors.append(average_color)

    return average_colors






def main():

    # Clear out the existing "images" folder
    shutil.rmtree("images", ignore_errors=True)

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

    # Now we go and call text ChatGPT
    response = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[
            {"role": "system", "content": "Generate discrete paragraphs from the prompt. "
                                          "They are being used in DAL-E to create images to along with is. Limit your"
                                          "response to 1000 tokens"},
            {"role": "assistant", "content": "Tell me a story about " + transcript.text},
        ]
    )

    chat_response = response.choices[0].message.content
    print("ChatGPT Response:")
    print(chat_response)

    # Break the response into paragraphs
    paragraphs = [p.strip() for p in chat_response.split('\n\n') if p.strip()]

    # Break the paragraphs into chunks of two
    chunk_size = 2
    paragraph_chunks = [paragraphs[i:i + chunk_size] for i in range(0, len(paragraphs), chunk_size)]

    image_urls = []
    image_octants = []

    # Iterate through paragraph chunks and generate images using Dal-E
    for index, chunk in enumerate(paragraph_chunks):
        combined_paragraphs = '\n\n'.join(chunk)
        print("Calling Image Generation for Paragraphs:", combined_paragraphs)
        try:
            response = client.images.generate(
                model="dall-e-3",
                prompt=combined_paragraphs,
                size="1024x1024",
                quality="standard",
                n=1,
            )

            # Process images
            image_url = response.data[0].url
            image_urls.append(image_url)

            # Create a folder for each picture based on the index
            folder_path = os.path.join("images", f"image_{index}")

            # Download and store the image
            image_path = download_image(image_url, folder_path, f"image_{index}.png")
            print("Downloaded and stored image:", image_path)

            # Calculate and print the average color values for each octant of the image
            average_colors = calculate_average_color(image_path)
            print("Average Color Values for Octants:", average_colors)
            image_octants.append(average_colors)

        except openai.OpenAIError as e:
            print("Error during Image Generation:", e.http_status, e.error)


    # create a .wav output for each of the story paragraphs
    sound_folder = Path(__file__).parent / "sound"
    shutil.rmtree(sound_folder, ignore_errors=True)
    sound_folder.mkdir(exist_ok=True)

    for index, paragraph in enumerate(paragraphs):
        speech_file_path = sound_folder / f"speech_{index}.wav"
        print(f"Generating speech for Paragraph {index + 1}...")
        text_to_speech(paragraph, speech_file_path)
        print(f"Speech generated and saved at: {speech_file_path}")



if __name__ == "__main__":
    main()
