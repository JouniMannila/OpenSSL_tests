object formMain: TformMain
  Left = 0
  Top = 0
  Caption = 'TCP Server'
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
  object butListen: TButton
    Left = 184
    Top = 368
    Width = 75
    Height = 25
    Caption = 'Listen'
    TabOrder = 0
    OnClick = butListenClick
  end
  object memo: TMemo
    Left = 0
    Top = 0
    Width = 624
    Height = 313
    Align = alTop
    TabOrder = 1
  end
  object Timer1: TTimer
    Interval = 200
    OnTimer = Timer1Timer
    Left = 32
    Top = 32
  end
end
