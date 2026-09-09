#include "pch.h"
#include "WeatherPro.h"
#include "afxdialogex.h"
#include "OptionsOmDlg.h"
#include "resource.h"

#include "Common.h"
#include "DataManager.h"

IMPLEMENT_DYNAMIC(OptionsOmDlg, CDialogEx)

OptionsOmDlg::OptionsOmDlg(CWnd* pParent /*=nullptr*/)
        : CDialogEx(IDD_DLG_OPTIONS_OM, pParent)
{
}

OptionsOmDlg::~OptionsOmDlg()
{
}

void OptionsOmDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialogEx::DoDataExchange(pDX);
        DDX_Control(pDX, IDC_COMBO_UNIT_TYPE, ctrl_combobox_unit_type);
        DDX_CBIndex(pDX, IDC_COMBO_UNIT_TYPE, int_unit_type);
}

BEGIN_MESSAGE_MAP(OptionsOmDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL OptionsOmDlg::OnInitDialog()
{
        CDialogEx::OnInitDialog();

        const auto &provider = DataManager::Instance().GetApiCollections().GetApiOpenMeteo().GetProviderOpenMeteo();

        ctrl_combobox_unit_type.AddString(cmn::GetStringRes(IDS_UNITS_METRIC));
        ctrl_combobox_unit_type.AddString(cmn::GetStringRes(IDS_UNITS_IMPERIAL));
        ctrl_combobox_unit_type.SetCurSel(static_cast<int>(provider.config.unit_type));

        UpdateData(FALSE);

        return TRUE;
}

void OptionsOmDlg::OnOK()
{
        UpdateData(TRUE);

        auto &api = DataManager::Instance().GetApiCollections().GetApiOpenMeteo();
        api.config.unit_type = static_cast<DataProviderOpenMeteo::UnitType>(int_unit_type);
        api.GetProviderOpenMeteo().config.unit_type = api.config.unit_type;

        DataManager::Instance().SaveConfigs();

        CDialogEx::OnOK();
}
