#pragma once

#include "afxdialogex.h"

class OptionsOmDlg : public CDialogEx
{
    DECLARE_DYNAMIC(OptionsOmDlg)

public:
    OptionsOmDlg(CWnd* pParent = nullptr);
    ~OptionsOmDlg() override;

    enum { IDD = IDD_DLG_OPTIONS_OM };

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    void OnOK() override;

    DECLARE_MESSAGE_MAP()

    CComboBox ctrl_combobox_unit_type;
    int int_unit_type{ 0 };

    BOOL OnInitDialog() override;
};
