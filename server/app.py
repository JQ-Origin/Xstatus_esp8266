from flask import Flask, jsonify
import requests
import datetime

app = Flask(__name__)

Almanac = {
    "gregorian_date": "",
    "lunar_date": "",
    "yi": "",
    "ji": "",
    "shen_wei": ""
}
Checktime = {
    "time":""
}
def update_time():
    now = datetime.datetime.now()
    Checktime["time"]=now.strftime('%Y-%m-%d')

def fetch_data_from_api():
    url1 = "https://www.36jxs.com/api/Commonweal/almanac?sun="+Checktime["time"]  # 替换为您的API地址
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
    }
    
    try:
        response = requests.get(url1, headers=headers)
        response.raise_for_status()  # 检查请求是否成功
        data = response.json()  # 将响应内容解析为JSON格式

        if data.get("code") == 1:
            date_info = data.get("data", {})
            # 提取所需信息
            Almanac["gregorian_date"] = date_info.get("GregorianDateTime")
            Almanac["lunar_date"] = date_info.get("LunarDateTime")
            Almanac["yi"] = date_info.get("Yi")
            Almanac["ji"] = date_info.get("Ji")
            Almanac["shen_wei"] = date_info.get("ShenWei")

    except requests.exceptions.RequestException as e:
        print(f"请求失败: {e}")
    except ValueError as e:
        print(f"解析JSON失败: {e}")

@app.route('/api/time', methods=['GET'])
def get_time():
    update_time()
    return jsonify(Checktime)  # 返回JSON格式的数据
@app.route('/api/almanac', methods=['GET'])
def get_almanac():
    update_time()
    fetch_data_from_api()  # 获取最新数据
    return jsonify(Almanac)  # 返回JSON格式的数据

if __name__ == '__main__':
    app.run(host="0.0.0.0",port=7575)  # 启动Flask应用
