object formMain: TformMain
  Left = 0
  Top = 0
  Caption = 'TLS_Client'
  ClientHeight = 441
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  OnShow = FormShow
  TextHeight = 15
  object memo: TMemo
    Left = 0
    Top = 57
    Width = 624
    Height = 384
    Align = alClient
    Color = 5723991
    TabOrder = 0
  end
  object panTop: TPanel
    Left = 0
    Top = 0
    Width = 624
    Height = 57
    Align = alTop
    Caption = 'Top'
    ShowCaption = False
    TabOrder = 1
    object butConnect: TBitBtn
      Left = 16
      Top = 16
      Width = 80
      Height = 25
      Caption = 'Connect'
      TabOrder = 0
      OnClick = butConnectClick
    end
    object butSend: TBitBtn
      Left = 110
      Top = 16
      Width = 80
      Height = 25
      Caption = 'Send'
      TabOrder = 1
      OnClick = butSendClick
    end
  end
  object timer: TTimer
    Enabled = False
    OnTimer = timerTimer
    Left = 32
    Top = 88
  end
end
