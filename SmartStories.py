# Configure local API key using https://www.datacamp.com/tutorial/converting-speech-to-text-with-the-openAI-whisper-API

from openai import OpenAI
import openai
import os
import requests
import shutil
import cv2
import numpy as np
from pathlib import Path
from gtts import gTTS

client = OpenAI(api_key='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')



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

    # Split the image into 4x4 grid
    height, width, _ = image.shape
    chunk_height, chunk_width = height // 4, width // 4

    # Calculate average color and store with chunk index
    average_colors_with_index = []

    # Top edge
    for i in range(4):
        chunk = image[0:chunk_height, i * chunk_width: (i + 1) * chunk_width, :]
        average_color = np.mean(chunk, axis=(0, 1))
        average_colors_with_index.append((average_color, i))

    # Right edge
    for i in range(1, 4):
        chunk = image[i * chunk_height: (i + 1) * chunk_height, -chunk_width:, :]
        average_color = np.mean(chunk, axis=(0, 1))
        average_colors_with_index.append((average_color, i + 3))

    # Bottom edge
    for i in range(2, -1, -1):
        chunk = image[-chunk_height:, i * chunk_width: (i + 1) * chunk_width, :]
        average_color = np.mean(chunk, axis=(0, 1))
        if (i == 2):
            average_colors_with_index.append((average_color, i + 5))
        elif (i == 1):
            average_colors_with_index.append((average_color, i + 7))
        elif (i == 0):
            average_colors_with_index.append((average_color, i + 9))

            # Left edge
    for i in range(2, 0, -1):
        chunk = image[i * chunk_height: (i + 1) * chunk_height, 0:chunk_width, :]
        average_color = np.mean(chunk, axis=(0, 1))
        if (i == 2):
            average_colors_with_index.append((average_color, i + 8))
        elif (i == 1):
            average_colors_with_index.append((average_color, i + 10))

    return average_colors_with_index








def save_color_values(folder_path, index, color_values):
    colorvalues_folder = os.path.join(folder_path, "colorvalues")
    os.makedirs(colorvalues_folder, exist_ok=True)

    file_path = os.path.join(colorvalues_folder, f"color_values_{index}.txt")
    with open(file_path, 'w') as file:
        for color in color_values:
            file.write(','.join(map(str, color)) + '\n')
    print("Color values saved to:", file_path)


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


    # create a .wav output for each of the story paragraphs
    sound_folder = Path(__file__).parent / "sound"
    shutil.rmtree(sound_folder, ignore_errors=True)
    sound_folder.mkdir(exist_ok=True)

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

            # Save each color value to a separate text file
            save_color_values(folder_path, index, average_colors)

            speech_file_path = sound_folder / f"speech_{index}.wav"
            print(f"Generating speech for Paragraph {index + 1}...")
            text_to_speech(combined_paragraphs, speech_file_path)
            print(f"Speech generated and saved at: {speech_file_path}")

        except openai.OpenAIError as e:
            print("Error during Image Generation:", e.http_status, e.error)




    #for index, paragraph in enumerate(paragraph_chunks):




if __name__ == "__main__":
    main()
