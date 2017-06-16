[170616追記:伊藤 変更履歴]

Task On/Off定義(TASK_ON/TASK_OFF)の追加 

Log task 以下関数の追加 
(不要の場合はLOG_TASKをTASK_OFFにすること)

- Log_str()
　グローバル配列 gst_Log_strに現在のセンサー値を格納
　引数、返り値無し
　Log_str()は繰り返し動作内で呼び出されることが期待

- Log_commit()グローバル配列 
　gst_Log_strに格納されているデータをファイル出力する
　引数、返り値無し
　Log_commit()は最後に1回呼び出されることが期待


Logの回数はLOG_MAXにて定義
Logのファイル名はLOG_FILE_NAMEにて定義