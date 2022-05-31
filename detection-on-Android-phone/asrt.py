import asrt_sdk

host = '10.119.11.20'
port = '80'
protocol = 'http'
speech_recognizer = asrt_sdk.get_speech_recognizer(host, port, protocol)

from new import record_main

def main():
    record_main('demo.wav')

    filename = './demo.wav'
    result = speech_recognizer.recognite_file(filename)
    # print(result)
#    print(result.result)

    wave_data = asrt_sdk.read_wav_datas(filename)
    result = speech_recognizer.recognite_speech(wave_data.str_data,
                                                wave_data.sample_rate,
                                                wave_data.channels,
                                                wave_data.byte_width)
    # print(result)
 #   print(result.result)
    try:
        for i in result.result:
            if("you" in i):
                return 1
            elif("zuo" in i):
                return 2
            elif("qian" in i):
                return 3
            elif("hou" in i):
                return 4
    except:
        return 0
    return 0


    result = speech_recognizer.recognite_language(result.result)
    # print(result)
    # print(result.result)


print(main())
