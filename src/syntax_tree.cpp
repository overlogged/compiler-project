#include "syntax_tree.h"

void syntax_analysis(node_module module)
{
    // 对于每一个可以定义函数的“环境”

    // 第一步：扫描所有类型定义，生成全局类型表（固定）
    // 需要封装类型表的功能，以支持 built-in 类型
    std::map<std::string, syntax_type> type_table;

    // 第二步：扫描所有函数定义，生成全局函数表（固定）
    // 需要封装函数表的功能，以支持 built-in 类型
    for (auto &block : module.blocks)
    {
        if (auto pfun_block = std::get_if<node_function_block>(&block))
        {
        }
    }

    // 第三步：扫描全局变量的声明，生成全局变量符号和类型定义，此处需要类型推导。生成初始化函数 __init

    // 第四步：进入每个 block，完成语义分析
}