object formMain: TformMain
  Left = 0
  Top = 0
  Caption = 'TLS Server'
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
  object Button1: TButton
    Left = 280
    Top = 232
    Width = 75
    Height = 25
    Caption = 'Button1'
    TabOrder = 0
    OnClick = Button1Click
  end
  object memo: TMemo
    Left = 8
    Top = 8
    Width = 608
    Height = 185
    TabOrder = 1
  end
end
