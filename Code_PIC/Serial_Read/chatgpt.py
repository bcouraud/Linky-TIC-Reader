from ollama import chat
from ollama import ChatResponse
response: ChatResponse = chat(model='gpt-oss:20b', messages = [
    {
        'role': 'User',
        'content': 'Is it better to keep heaters always on, or to cut the heating and wait to be back from work to heat it?',
        # 'content': 'write a poem of 4 sentences'
        # 'content': 'Why does the local model running on Ollama takes ages to anwser my question ?'
    },
])

print(response['message']['content'])
print(response.message.content)

