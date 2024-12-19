object formMain: TformMain
  Left = 0
  Top = 0
  Caption = 'OpenSSL'
  ClientHeight = 457
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  OnShow = FormShow
  DesignSize = (
    624
    457)
  TextHeight = 15
  object butEncrypt: TButton
    Left = 112
    Top = 408
    Width = 75
    Height = 25
    Anchors = [akBottom]
    Caption = 'Encrypt'
    TabOrder = 0
    OnClick = butEncryptClick
  end
  object butDecrypt: TButton
    Left = 408
    Top = 408
    Width = 75
    Height = 25
    Anchors = [akBottom]
    Caption = 'Decrypt'
    TabOrder = 1
    OnClick = butDecryptClick
  end
  object memo1: TMemo
    Left = 8
    Top = 12
    Width = 608
    Height = 181
    Anchors = [akLeft, akTop, akRight]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    Lines.Strings = (
      'The quick brown fox jumps over the lazy dog.')
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 2
  end
  object memo2: TMemo
    Left = 8
    Top = 208
    Width = 608
    Height = 161
    Anchors = [akLeft, akTop, akRight]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 3
  end
end
