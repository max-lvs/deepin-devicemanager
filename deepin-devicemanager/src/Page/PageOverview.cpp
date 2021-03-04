// 项目自身文件
#include "PageOverview.h"

// Qt库文件
#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QAction>
#include <QDebug>
#include <QClipboard>

// Dtk头文件
#include <DApplication>
#include <DFontSizeManager>
#include <DMenu>
#include <DSysInfo>
#include <DMessageManager>
#include <DNotifySender>
#include <DTextBrowser>

// 其它头文件
#include "PageTableWidget.h"
#include "DeviceManager.h"
#include "PageInfoWidget.h"
#include "LongTextLabel.h"
#include "MacroDefinition.h"

#define ENTER_ONE 60    // 换行的位置 1
#define ENTER_TWO 120   // 换行的位置 2                                                                    // end html

PageOverview::PageOverview(DWidget *parent)
    : PageInfo(parent)
    , mp_PicLabel(new DLabel(this))
    , mp_DeviceLabel(new LongTextLabel(this))
    , mp_OSLabel(new LongTextLabel(this))
    , mp_Overview(new PageTableWidget(this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export"), this))
    , mp_Copy(new QAction(/*QIcon::fromTheme("edit-copy"), */tr("Copy"), this))
    , mp_Menu(new DMenu(this))
{
    // 初始化界面布局
    initWidgets();

    // 设置右键按钮策略
    mp_Overview->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接槽函数
    connect(mp_Overview, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageOverview::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &PageOverview::slotActionExport);
    connect(mp_Copy, &QAction::triggered, this, &PageOverview::slotActionCopy);
}

void PageOverview::updateInfo(const QList<DeviceBaseInfo *> &)
{

}

void PageOverview::updateInfo(const QMap<QString, QString> &map)
{
    mp_Overview->clear();

    int row = map.size();

    // 根据页面高度确定表格最多显示行数
    int maxRow = this->height() / ROW_HEIGHT - 4;
    if (maxRow < 1) {
        mp_Overview->setLimitRow(11);

    } else {
        mp_Overview->setLimitRow(std::min(11, maxRow));
    }

    // 设置表格行数
    mp_Overview->setColumnAndRow(row - 1);

    int i = 0;

    const QList<QPair<QString, QString>> types = DeviceManager::instance()->getDeviceTypes();

    foreach (auto iter, types) {
        if (iter.first == tr("Overview")) {
            continue;
        }

        // 按设备类型列表顺序显示概况信息
        if (map.find(iter.first) != map.end()) {
            QTableWidgetItem *itemFirst = new QTableWidgetItem(iter.first);
            mp_Overview->setItem(i, 0, itemFirst);
            QTableWidgetItem *itemSecond = new QTableWidgetItem(map.find(iter.first).value());
            mp_Overview->setItem(i, 1, itemSecond);
            ++i;
        }
    }
}

void PageOverview::clearWidgets()
{
    return;
}

void PageOverview::setLabel(const QString &itemstr)
{

}

void PageOverview::setLabel(const QString &str1, const QString &str2)
{
    // 打开超链接属性
    mp_OSLabel->setOpenExternalLinks(true);

    // 设置ToolTip
    QString tips = str2;
    if (tips.length() > ENTER_ONE)
        tips.insert(ENTER_ONE, QChar('\n'));
    if (tips.length() > ENTER_TWO)
        tips.insert(ENTER_TWO, QChar('\n'));

    mp_OSLabel->setToolTip(tips);

    // 超过控件长度用...代替
    QString os = str2;
    QString linkStr = LINK_STR;

    // 系统类型+链接
    DSysInfo::UosEdition type = DSysInfo::uosEditionType();
    if (DSysInfo::UosProfessional == type) { // 桌面专业版
        linkStr += PROF_STR + END_STR + os.remove(PROF_STR);
    } else if (DSysInfo::UosHome == type) {  // 个人版
        linkStr += HOME_STR + END_STR + os.remove(HOME_STR);
    } else if (DSysInfo::UosCommunity == type) { // 社区版
        linkStr = COMMUNITY_LINK_STR;  // 社区版的链接与其它的不同
        linkStr += COMMUNITY_STR + END_STR + os.remove(COMMUNITY_STR);
    }
#if(DTK_VERSION > DTK_VERSION_CHECK(5,4,10,0))
    else if (DSysInfo::UosEducation == type) {// 教育版
        linkStr += EDUC_STR + END_STR + os.remove(EDUC_STR);
    }
#endif
    else if (DSysInfo::UosEnterprise == type) {// 服务器企业版
        linkStr += ENTERPRISE_STR + END_STR + os.remove(ENTERPRISE_STR);
    } else if (DSysInfo::UosEnterpriseC == type) {// 服务器行业版
        linkStr += ENTERPRISEC_STR + END_STR + os.remove(ENTERPRISEC_STR);
    } else if (DSysInfo::UosEuler == type) {// 服务器欧拉版
        linkStr += EULER_STR + END_STR + os.remove(EULER_STR);
    } else {// 默认值
        linkStr += DEFAULT_STR + END_STR + os.remove(DEFAULT_STR);
    }

    // 设置系统描述
    mp_OSLabel->setText(linkStr);

    // 设置设备信息
    mp_DeviceLabel->setText(str1);

    // 设备信息加粗
    QFont font = mp_DeviceLabel->font();
    font.setWeight(63);
    mp_DeviceLabel->setFont(font);

    // 设置字号
    DFontSizeManager::instance()->bind(mp_DeviceLabel, DFontSizeManager::T5);

    // 资源文件获取
    QString path = "";
    if (str1.contains("desktop", Qt::CaseInsensitive)) {
        path = "device_desktop";
    } else if (str1.contains("laptop", Qt::CaseInsensitive) ||
               str1.contains("notebook", Qt::CaseInsensitive)) {
        path = "device_laptop";
    } else if (str1.contains("ternimal", Qt::CaseInsensitive)) {
        path = "device_terminal";
    } else if (str1.contains("Tablet", Qt::CaseInsensitive)) {
        path = "device_tablet";
    } else if (str1.contains("server", Qt::CaseInsensitive)) {
        path = "device_server";
    } else {
        path = "device_desktop";
    }

    // 设置计算器图片
    QIcon icon(QIcon::fromTheme(path));
    QPixmap pic = icon.pixmap(96, 96);
    mp_PicLabel->setPixmap(pic);
}

void PageOverview::slotShowMenu(const QPoint &)
{
    // 右键菜单
    mp_Menu->clear();
    mp_Menu->addAction(mp_Copy);
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    mp_Menu->exec(QCursor::pos());
}

void PageOverview::slotActionRefresh()
{
    // 刷新
    emit refreshInfo();
}

void PageOverview::slotActionExport()
{
    qDebug() << "export";
    // 导出
    emit exportInfo();
}

void PageOverview::slotActionCopy()
{
    // 拷贝
    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setText(mp_Overview->toString());
}

void PageOverview::initWidgets()
{
    // 初始化页面布局
    mp_OSLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mp_DeviceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 新建Layout不要给定父窗口
    QVBoxLayout *vLayout = new QVBoxLayout();

    // 上部分
    QHBoxLayout *uphLayout = new QHBoxLayout();

    // 上左
    uphLayout->addWidget(mp_PicLabel);
    // 上右
    QVBoxLayout *uplvLayout = new QVBoxLayout();
    uplvLayout->setContentsMargins(0, 0, 0, 0);
    uplvLayout->addSpacing(15);
    uplvLayout->addWidget(mp_DeviceLabel);
    uplvLayout->addSpacing(5);
    uplvLayout->addWidget(mp_OSLabel);
    uplvLayout->addSpacing(15);
    uphLayout->addLayout(uplvLayout);
    vLayout->addLayout(uphLayout);

    // 下部分
    vLayout->addWidget(mp_Overview);
    vLayout->addStretch();
    setLayout(vLayout);
}
