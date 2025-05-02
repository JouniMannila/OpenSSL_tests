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
  TextHeight = 15
  object memo: TMemo
    Left = 0
    Top = 57
    Width = 624
    Height = 384
    Align = alClient
    Color = 5723991
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 13695988
    Font.Height = -12
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
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
    DesignSize = (
      624
      57)
    object lblAdddress: TLabel
      Left = 356
      Top = 20
      Width = 42
      Height = 15
      Anchors = [akTop, akRight]
      Caption = 'Address'
    end
    object lblPort: TLabel
      Left = 501
      Top = 20
      Width = 22
      Height = 15
      Anchors = [akTop, akRight]
      Caption = 'Port'
    end
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
    object edAddress: TEdit
      Left = 408
      Top = 16
      Width = 77
      Height = 23
      Anchors = [akTop, akRight]
      TabOrder = 2
      Text = '172.20.221.88'
    end
    object edPort: TEdit
      Left = 532
      Top = 16
      Width = 41
      Height = 23
      Anchors = [akTop, akRight]
      TabOrder = 3
      Text = '10001'
    end
    object udPort: TUpDown
      Left = 573
      Top = 16
      Width = 17
      Height = 23
      Anchors = [akLeft, akRight]
      Associate = edPort
      Max = 65535
      Position = 10001
      TabOrder = 4
      Thousands = False
    end
  end
  object timer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = timerTimer
    Left = 32
    Top = 88
  end
end
