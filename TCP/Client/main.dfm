object formMain: TformMain
  Left = 0
  Top = 0
  Caption = 'TCP Client'
  ClientHeight = 441
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object butConnect: TButton
    Left = 272
    Top = 376
    Width = 75
    Height = 25
    Caption = 'Connect'
    TabOrder = 0
    OnClick = butConnectClick
  end
  object memo: TMemo
    Left = 0
    Top = 0
    Width = 624
    Height = 305
    Align = alTop
    TabOrder = 1
  end
end
